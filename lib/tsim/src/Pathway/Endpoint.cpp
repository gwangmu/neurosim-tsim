#include <TSim/Pathway/Endpoint.h>

#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Module/Module.h>
#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <queue>

using namespace std;


Endpoint *Endpoint::_PORTCAP = nullptr;


Endpoint::Endpoint (string name, Pathway *parent, Type type, 
        uint32_t capacity, PERMIT(Pathway))
    : Metadata ("Endpoint", name)
{
    this->type = type;

    if (!parent)
        SYSTEM_ERROR ("endpoint '%s' with null parent", name.c_str());
    this->parent = parent;

    this->modConn = nullptr;
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

    if (capacity == 0 && type == RHS)
        DESIGN_FATAL ("zero-capacity RHS endpoint not allowed", GetName().c_str());

    if (capacity == 1 && type == LHS)
        DESIGN_WARNING ("capacity==1 LHS endpoint may merely add one-cycle delay",
                GetName().c_str());

    this->capacity = capacity;
}


void Endpoint::Reserve ()
{
    if (unlikely (type == CAP))
        SIM_WARNING ("meaningless action reserving PORTCAP", GetName().c_str());

    resv_count++;

    if (unlikely (resv_count + msgque.size() > capacity))
        SYSTEM_ERROR ("queue size + resv_count exceeded capacity");
}

bool Endpoint::Assign (Message *msg)
{
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

    if (unlikely (capacity != 0 && msgque.size () == capacity))
    {
        SIM_WARNING ("attempted to enque to full endpoint", GetName().c_str());
        return false;
    }
    else if (unlikely (capacity == 0 && msgque.size () >= 1))
    {
        SYSTEM_ERROR ("capacity=0 endpoint cannot have elements more than one");
        return false;
    }
    
    if (unlikely (msg == nullptr))
        SYSTEM_ERROR ("attempted to push null message");

    if (resv_count > 0) resv_count--;
    msgque.push (msg);

    return true;
}

bool Endpoint::IsFull () 
{ 
    if (unlikely (msgque.size () > capacity))
        SYSTEM_ERROR ("endpoint queue size exceeded capacity");

    return (resv_count + msgque.size()) >= capacity;
}


bool Endpoint::JoinTo (Module *module, string portname, PERMIT(Module))
{
    if (modConn != nullptr)
        DESIGN_WARNING ("port '%s' (of %s) has been already assigned",
                GetName().c_str(), portname.c_str(), module->GetName().c_str());

    modConn = module;
    portConn = portname;
    return true;
}
