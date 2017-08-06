#include <TSim/Pathway/Endpoint.h>

#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Pathway/NullMessage.h>
#include <TSim/Module/Module.h>
#include <TSim/Device/Device.h>
#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>
#include <TSim/Utility/Prototype.h>

#include <string>
#include <cinttypes>
#include <queue>

using namespace std;
using namespace TSim;


Endpoint *Endpoint::_PORTCAP = nullptr;


Endpoint::Endpoint (string name, uint32_t id, Pathway *parent, Type type, 
        uint32_t capacity, PERMIT(Pathway))
    : Metadata ("Endpoint", name)
{
    this->type = type;
    this->id = id;

    if (!parent)
        SYSTEM_ERROR ("endpoint '%s' with null parent", name.c_str());
    this->parent = parent;
    msgproto = parent->GetMsgPrototype ();

    this->unitConn = nullptr;
    this->portConn = "";

    if (capacity == 0 && type == RHS)
        DESIGN_FATAL ("zero-capacity RHS endpoint not allowed", GetName().c_str());
    this->capacity = capacity;

    this->resv_count = 0;
    this->selected_lhs = false;
}
    
Endpoint::Endpoint () : Metadata ("Endpoint", "PORTCAP")
{
    type = CAP; 
    capacity = 0;
    resv_count = 0;
    msgproto = Prototype<NullMessage>::Get ();
    selected_lhs = 0;
    parent = nullptr; 
}

void Endpoint::SetCapacity (uint32_t capacity)
{
    if (type == CAP)
    {
        DESIGN_WARNING ("cannot set capacity of PORTCAP", GetName().c_str());
        return;
    }

    //if (capacity == 0 && type == RHS)
    //    DESIGN_FATAL ("zero-capacity RHS endpoint not allowed", GetName().c_str());

    if (capacity == 1 && type == LHS)
        DESIGN_WARNING ("capacity==1 LHS endpoint may merely add one-cycle delay",
                GetName().c_str());

    if (parent->GetMsgPrototype()->GetType() == Message::TOGGLE &&
            type == LHS && capacity != 0)
        DESIGN_ERROR ("RHS having TOGGLE message prototype does not make sense",
                GetName().c_str());

    if (parent->GetMsgPrototype()->GetType() == Message::TOGGLE &&
            type == RHS && capacity > 1)
        DESIGN_ERROR ("RHS having TOGGLE message prototype does not make sense",
                GetName().c_str());

    if (capacity == 0 && parent->GetMsgPrototype()->BIT_WIDTH > parent->GetBitWidth())
        DESIGN_FATAL ("pathway narrower than message width cannot have capacity=0 endpoint",
                GetName().c_str());

    this->capacity = capacity;
}


void Endpoint::Reserve ()
{
    if (unlikely (type == CAP))
        SIM_WARNING ("meaningless action reserving PORTCAP", GetName().c_str());

    resv_count += 4;

    if (unlikely (resv_count + msgque.size() > capacity))
        SYSTEM_ERROR ("[%s] queue size + resv_count exceeded capacity",
                this->GetConnectedUnit()->GetName().c_str());
}

bool Endpoint::Assign (Message *msg)
{
    if (!msg)
    {
        SIM_WARNING ("attempted to push null message (path: %s)", 
                GetName().c_str(), GetParent()->GetName().c_str());
        return false;
    }

    if (unlikely (type == CAP))
    {
        if (msg)
        {
            PRINT ("port '%s' received message %s (%p)",
                    portConn.c_str(), msg->GetClassName(), msg);
            msg->Dispose ();
        }
        return true;
    }
    
    // NOTE: if TOGGLE type, clean up pending msg
    if (msg->GetType() == Message::TOGGLE && !msgque.empty())
    {
        msgque.front()->Dispose ();
        msgque.pop ();
    }


    if (unlikely (capacity != 0 && msgque.size () == capacity))
    {
        SIM_WARNING ("attempted to enque to full endpoint", GetName().c_str());
        return false;
    }
    else if (unlikely (capacity == 0 && msgque.size () >= 1))
    {
        SYSTEM_ERROR ("capacity=0 endpoint cannot have elements more than one "
                "(path: %s)", GetParent()->GetName().c_str());
        return false;
    }
    
    if (unlikely (msg == nullptr))
        SYSTEM_ERROR ("attempted to push null message");

    if (resv_count > 0) resv_count--;

    msgque.push (msg);

    DEBUG_PRINT ("assining message %p (%s - %s)",
            msg, GetConnectedUnit()->GetName().c_str(), GetConnectedPortName().c_str());

    return true;
}


bool Endpoint::IsFull () 
{ 
    if (unlikely (msgque.size () > capacity))
        SYSTEM_ERROR ("endpoint queue size exceeded capacity");

    return (resv_count + msgque.size()) >= capacity;
}

bool Endpoint::IsOverloaded ()
{
    if (unlikely (capacity != 0 && resv_count + msgque.size () > capacity))
        SYSTEM_ERROR ("capacity!=0 endpoint cannot exceed capacity");
    else if (unlikely (capacity == 0 && resv_count + msgque.size () > 1))
        SYSTEM_ERROR ("capacity==0 endpoint(%s) cannot overload by more than 1 (now: %d, %zu)"
                , this->GetConnectedPortName().c_str(), resv_count, msgque.size());

    return (resv_count + msgque.size () > capacity);
}

void Endpoint::SetExtReadyState (bool val)
{
    parent->SetExtReadyState (id, val);
}

bool Endpoint::IsBroadcastBlocked ()
{
    return parent->IsBroadcastBlocked (id);
}


bool Endpoint::JoinTo (Component *comp, string portname)
{
    if (Unit *unit = dynamic_cast<Unit *>(comp))
    {
        if (unitConn != nullptr)
            DESIGN_WARNING ("already assigned endpoint", GetName().c_str());

        unitConn = unit;
        portConn = portname;
    }
    else
    {
        SYSTEM_ERROR ("non-unit component tries to join to endpoint");
        return false;
    }

    return true;
}
