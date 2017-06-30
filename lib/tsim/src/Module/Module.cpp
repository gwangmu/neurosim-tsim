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

    if (pdepth > MAX_PDEPTH)
        DESIGN_FATAL ("pdepth > %u not allowed", GetName().c_str(), MAX_PDEPTH);

    omsgidxmask = 1;
    for (uint32_t pd = pdepth; pd; omsgidxmask <<= 1, pd >>= 1);
    omsgidxmask--;

    nextoutmsgs = new Message **[omsgidxmask + 1];
    for (uint32_t i = 0; i < omsgidxmask + 1; i++)
        nextoutmsgs[i] = new Message *[1];

    outque_size = new uint32_t[0];

    ninports = noutports = 0;

    pbusy_state = 0;

    dynpower = -1;
    stapower = -1;
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
                GetFullName().c_str(), 
                endpt->GetParent()->GetMsgPrototype()->GetClassName (),
                endpt->GetParent()->GetName().c_str(), 
                port->msgproto->GetClassName (), portname.c_str();
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

Component::CycleClass<double> Module::GetAggregateCycleClass ()
{
    CycleClass<double> cclass_conv;
    cclass_conv.active = cclass.active;
    cclass_conv.idle = cclass.idle;

    return cclass_conv;
}

Component::EventCount<double> Module::GetAggregateEventCount ()
{
    EventCount<double> ecount_conv;
    ecount_conv.stalled = ecount.stalled;
    // NOTE: ecount.oport_full is non-aggregatable

    return ecount_conv;
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

    if (dynpower == -1)
    {
        DESIGN_WARNING ("no dynamic power info", GetFullName().c_str());
        icount.warning++;
    }

    if (stapower == -1)
    {
        DESIGN_WARNING ("no static power info", GetFullName().c_str());
        icount.warning++;
    }

    for (auto i = 0; i < ninports; i++)
    {
        Port &port = inports[i];
        if (!port.endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
            icount.warning++;
        }
    }
    
    for (auto i = 0; i < noutports; i++)
    {
        Port &port = outports[i];
        if (!port.endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
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

        for (auto p = 0; p < noutports; p++)
        {
            Port &outport = outports[p];

            if (outport.endpt->GetCapacity() == 0)
            {
                if (!outport.endpt->IsSelectedLHSOfThisCycle ()
                        || outport.endpt->IsOverloaded ())
                {
                    DEBUG_PRINT("%s is stalled (capacity %d)"
                            , outport.endpt->GetConnectedPortName().c_str(), outport.endpt->GetCapacity());
                    stalled = true;
                    break;
                }
            }
            else if (outport.endpt->IsFull ())
            {
                DEBUG_PRINT("%s is full (stall)", GetFullName().c_str());
                stalled = true;
                ecount.oport_full[p]++;
                break;
            }
        }
    }

    if (likely (pdepth == 0 || !stalled))
    {
        operation ("assign module output to pathway")
        {
            for (auto i = 0; i < noutports; i++)
            {
                if (nextoutmsgs[omsgidx][i])
                {
                    if (pdepth == 0)
                    {
                        if (Message::IsReserveMsg (nextoutmsgs[omsgidx][i]))
                        {
                            outports[i].endpt->Reserve ();
                            nextoutmsgs[omsgidx][i] = nullptr;
                            continue;
                        }
                    }

                    if (unlikely (pdepth != 0 && 
                            Message::IsReserveMsg (nextoutmsgs[omsgidx][i])))
                        SIM_FATAL ("pdepth!=0 cannot produce RESERVE msg",
                                GetName().c_str());
                    
                    bool assnres = outports[i].endpt->Assign 
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
    MICRODEBUG_PRINT ("calc '%s' (stall: %d)"
            , GetFullName().c_str(), stalled);

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
            for (auto i = 0; i < ninports; i++)
            {
                if (!nextinmsgs[i])
                {
                    nextinmsgs[i] = inports[i].endpt->Peek ();
                    DEBUG_PRINT ("peaking message %p", nextinmsgs[i]);
                }
            }
        }

        for (auto i = 0; i < noutports; i++)
            outque_size[i] = outports[i].endpt->GetNumMessages ();
    }

    if (likely (pdepth == 0 || !stalled))
    {
        operation ("call operation")
        {
            // NOTE: set nextinmsgs[i] to nullptr not to use ith input
            // NOTE: assign new message to nextoutmsgs[j] to push to jth output
            uint32_t omsgidx_out = (omsgidx + pdepth) & omsgidxmask;
            Operation (nextinmsgs, nextoutmsgs[omsgidx_out], 
                    outque_size, nextinstr);

            for (auto i = 0; i < noutports; i++)
            {
                if (nextoutmsgs[omsgidx_out])
                    MarkBusyPipeline ();
                break;
            }
        }
        
        operation ("pop used messages from RHS")
        {
            for (auto i = 0; i < ninports; i++)
            {
                if (nextinmsgs[i])
                {
                    inports[i].endpt->Pop ();
                    nextinmsgs[i]->Dispose ();
                    nextinmsgs[i] = nullptr;
                }
            }
        }

        omsgidx = (omsgidx + 1) & omsgidxmask;
        CommitPipeline ();
    }

    operation ("status check")
    {
        // NOTE: if (all_empty (nextoutmsgs)) at this point, then idle
        //  else if (stalled), then inactive but forced to stop (stalled)
        //  else active
        if (IsIdle () || stalled)   cclass.idle++;
        else                        cclass.active++;

        if (stalled) ecount.stalled++;
    }

    if (likely (pdepth == 0 || !stalled))
    {
        // TODO: optimize this
        for (auto i = 0; i < noutports; i++)
        {
            if (nextoutmsgs[omsgidx][i] && outports[i].endpt->GetCapacity() == 0)
            {
                operation ("ahead-of-time assign if LHS.capacity==0")
                {
                    if (unlikely (Message::IsReserveMsg (nextoutmsgs[omsgidx][i])))
                        SIM_FATAL ("capacity=0 endpoint cannot reserve",
                                GetName().c_str());

                    bool assnres = outports[i].endpt->Assign (nextoutmsgs[omsgidx][i]);
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
    Port port = Port();
    
    port.name = portname;
    port.iotype = iotype;
    port.msgproto = msgproto;
    port.endpt = nullptr;

    if (iotype == Module::PORT_INPUT)
    {
        if (ninports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#lhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = ninports++;
        port.id = id;
        inports[id] = port;
        pname2port.insert (make_pair (portname, &inports[id]));

        delete[] nextinmsgs;
        nextinmsgs = new Message *[ninports] ();
    }
    else if (iotype == Module::PORT_OUTPUT)
    {
        if (noutports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#rhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = noutports++;
        port.id = id;
        outports[id] = port;
        pname2port.insert (make_pair (portname, &outports[id]));

        for (uint32_t i = 0; i < omsgidxmask + 1; i++)
        {
            delete[] nextoutmsgs[i];
            nextoutmsgs[i] = new Message *[noutports] ();
        }
        
        delete[] outque_size;
        outque_size = new uint32_t[noutports] ();
    }
    else
    {
        DESIGN_ERROR ("cannot create %s port", GetFullName().c_str(),
                Module::Port::GetTypeString (iotype));
        return -1;
    }

    return id;
}
