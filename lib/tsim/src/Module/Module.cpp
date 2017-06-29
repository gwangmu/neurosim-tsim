#include <TSim/Module/Module.h>

#include <TSim/Base/Component.h>
#include <TSim/Script/Script.h>
#include <TSim/Register/Register.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <vector>
#include <string>
#include <map>

using namespace std;


/* Constructors */
Module::Module (const char *clsname, string iname, 
        Component *parent, uint32_t pdepth):
    Component (clsname, iname, parent)
{
    script = nullptr;
    reg = nullptr;
    stalled = false;

    this->pdepth = pdepth;
    omsgidx = 0;

    omsgidxmask = 1;
    for (uint32_t pd = pdepth; pd; omsgidxmask <<= 1, pd >>= 1);
    omsgidxmask--;

    nextoutmsgs = new Message **[omsgidxmask + 1];
    for (uint32_t i = 0; i < omsgidxmask + 1; i++)
        nextoutmsgs[i] = new Message *[1];

    //inports.resize (64);
    //outports.resize (64);
    inidx = outidx = 0;
}

Module::~Module ()
{
    for (int i=0; i<inports.size(); i++)
        delete inports[i];     
    for (int i=0; i<outports.size(); i++)
        delete outports[i];     
}


/* functions for 'Component' */
bool Module::SetScript (Script *script)
{
    if (this->script)
        DESIGN_WARNING ("overwriting '%s' with '%s'",
                GetFullName().c_str(), this->script->GetName().c_str(), 
                script->GetName().c_str());

    if (script->GetParent ()) {
        DESIGN_ERROR ("'%s' has already been assigned",
                GetFullName().c_str(), script->GetName().c_str());
        return false;
    }

    script->SetParent (this, KEY(Module));
    this->script = script;

    return true;
}

bool Module::SetRegister (Register *reg)
{
    if (this->reg)
        DESIGN_WARNING ("overwriting '%s' with '%s'",
                GetFullName().c_str(), this->reg->GetName().c_str(),
                reg->GetName().c_str());

    if (reg->GetParent ()) {
        DESIGN_ERROR ("'%s' has already been assigned",
                GetFullName().c_str(), reg->GetName().c_str());
        return false;
    }

    reg->SetParent (this, KEY(Module));
    this->reg = reg;

    return true;
}


bool Module::Connect (string portname, Endpoint *endpt)
{
    if (!pname2port.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'", GetFullName().c_str(), 
                portname.c_str());
        return false;
    }

    Port *port = pname2port[portname];

    if (port->iotype == Module::PORT_UNKNOWN)
    {
        SYSTEM_ERROR ("port type cannot be UNKNOWN"
                "(portname: %s)", portname.c_str());
        return false;
    }

    if (!endpt)
    {
        DESIGN_WARNING ("attemping to assign a null endpoint to the port '%s'",
                GetFullName().c_str(), portname.c_str());
        port->endpt = nullptr;
        return true;
    }

    if (!endpt->IsPortCap() && 
            endpt->GetParent()->GetMsgPrototype() != port->msgproto)
    {
        DESIGN_ERROR ("mismatching message proto %s (of '%s') "
                "and %s (of port '%s')",
                GetFullName().c_str(), 
                endpt->GetParent()->GetMsgPrototype()->GetClassName (),
                endpt->GetParent()->GetName().c_str(), 
                port->msgproto->GetClassName (), portname.c_str());
        return false;
    }

    if (!(endpt->GetEndpointType() == Endpoint::CAP) &&
            !(port->iotype == Module::PORT_INPUT &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Module::PORT_OUTPUT &&
                endpt->GetEndpointType() == Endpoint::LHS))
    {
        DESIGN_ERROR ("incompatible endpoint '%s' to %s port '%s'",
                GetName().c_str(), endpt->GetClassName(),
                Module::Port::GetTypeString(port->iotype), portname.c_str());
        return false;
    }

    port->endpt = endpt;
    if (!endpt->IsPortCap())
        endpt->JoinTo (this, portname, KEY(Module));

    return true;
}


Module* Module::GetModule (string name)
{
    if (GetInstanceName() == name)
        return this;
    else
        return nullptr;
}


/* functions for 'Simulator' */
IssueCount Module::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!GetParent())
    {
        SYSTEM_ERROR ("module '%s' has no parent", GetFullName().c_str());
        icount.error++;
    }

    for (Port *port : inports)
    {
        if (!port->endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port->name.c_str());
            icount.warning++;
        }
    }
    
    for (Port *port : outports)
    {
        if (!port->endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port->name.c_str());
            icount.warning++;
        }
    }

    return icount;
}

/*>>> ! PERFORMANCE-CRITICAL ! <<<*/
void Module::PreClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("assign '%s'-->path", GetFullName().c_str());

    operation ("update stall state")
    {
        stalled = false;

        for (Port *outport: outports)
        {
            if (outport->endpt->GetCapacity() == 0)
            {
                if (!outport->endpt->IsSelectedLHSOfThisCycle ()
                        || outport->endpt->IsOverloaded ())
                {
                    stalled = true;
                    break;
                }
            }
            else if (outport->endpt->IsFull ())
            {
                stalled = true;
                break;
            }
        }
    }

    if (likely (pdepth == 0 || !stalled))
    {
        operation ("assign module output to pathway")
        {
            for (auto i = 0; i < outports.size(); i++)
            {
                if (nextoutmsgs[omsgidx][i])
                {
                    if (pdepth == 0)
                    {
                        if (Message::IsReserveMsg (nextoutmsgs[omsgidx][i]))
                        {
                            outports[i]->endpt->Reserve ();
                            nextoutmsgs[omsgidx][i] = nullptr;
                            continue;
                        }
                    }

                    if (unlikely (pdepth != 0 && 
                            Message::IsReserveMsg (nextoutmsgs[omsgidx][i])))
                        SIM_FATAL ("pdepth!=0 cannot produce RESERVE msg",
                                GetName().c_str());
                    
                    bool assnres = outports[i]->endpt->Assign 
                        (nextoutmsgs[omsgidx][i]);
                    if (unlikely (!assnres))
                        SYSTEM_ERROR ("attemped to push to full RHS queue.");

                    nextoutmsgs[omsgidx][i] = nullptr;
                }
            }
        }
    }
}

/*>>> ! PERFORMANCE-CRITICAL ! <<<*/
void Module::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("calc '%s'", GetFullName().c_str());

    Instruction *nextinstr = nullptr;
    if (likely (!stalled))
    {
        if (script)
        {
            operation ("prepare instruction");
            nextinstr = script->NextInstruction ();
        }

        operation ("peak messages from RHS")
        {
            for (auto i = 0; i < inports.size(); i++)
                if (!nextinmsgs[i])
                {
                    nextinmsgs[i] = inports[i]->endpt->Peek ();
                    DEBUG_PRINT ("peaking message %p", nextinmsgs[i]);
                }
        }
    }

    if (likely (pdepth == 0 || !stalled))
    {
        operation ("call operation")
        {
            // NOTE: set nextinmsgs[i] to nullptr not to use ith input
            // NOTE: assign new message to nextoutmsgs[j] to push to jth output
            uint32_t omsgidx_out = (omsgidx + pdepth) & omsgidxmask;
            Operation (nextinmsgs, nextoutmsgs[omsgidx_out], nextinstr);
        }
        
        operation ("pop used messages from RHS")
        {
            for (auto i = 0; i < inports.size(); i++)
            {
                if (nextinmsgs[i])
                {
                    inports[i]->endpt->Pop ();
                    nextinmsgs[i]->Dispose ();
                    nextinmsgs[i] = nullptr;
                }
            }
        }

        omsgidx = (omsgidx + 1) & omsgidxmask;
    }

    // NOTE: if (stalled || all_empty (nextoutmsgs)) at this point,
    //  then this is in idle state

    if (likely (pdepth == 0 || !stalled))
    {
        // TODO: optimize this
        for (auto i = 0; i < outports.size(); i++)
        {
            if (nextoutmsgs[omsgidx][i] && outports[i]->endpt->GetCapacity() == 0)
            {
                operation ("ahead-of-time assign if LHS.capacity==0")
                {
                    if (unlikely (Message::IsReserveMsg (nextoutmsgs[omsgidx][i])))
                        SIM_FATAL ("capacity=0 endpoint cannot reserve",
                                GetName().c_str());

                    bool assnres = outports[i]->endpt->Assign (nextoutmsgs[omsgidx][i]);
                    if (unlikely (!assnres))
                        SYSTEM_ERROR ("attempted to push to full RHS");

                    nextoutmsgs[omsgidx][i] = nullptr;
                }
            }
        }
    }
}


/* Called by 'Module' */
uint32_t Module::CreatePort (string portname, Module::PortType iotype,
        Message* msgproto)
{
    uint32_t id = -1;
    Port *port = new Port();
    
    port->name = portname;
    port->iotype = iotype;
    port->msgproto = msgproto;
    port->endpt = nullptr;
    port->sealed = false;

    if (iotype == Module::PORT_INPUT)
    {
        id = inidx++;
        port->id = id;
        inports.push_back(port);

        delete[] nextinmsgs;
        nextinmsgs = new Message *[inports.size ()] ();
    }
    else if (iotype == Module::PORT_OUTPUT)
    {
        id = outidx++;
        port->id = id;
        outports.push_back(port);

        for (uint32_t i = 0; i < omsgidxmask + 1; i++)
        {
            delete[] nextoutmsgs[i];
            nextoutmsgs[i] = new Message *[outports.size ()] ();
        }
    }
    else
    {
        DESIGN_ERROR ("cannot create %s port", GetFullName().c_str(),
                Module::Port::GetTypeString (iotype));
        return -1;
    }

    pname2port.insert (make_pair (portname, port));

    return id;
}
