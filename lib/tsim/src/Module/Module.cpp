#include <TSim/Module/Module.h>

#include <TSim/Base/Component.h>
#include <TSim/Base/Unit.h>
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
    Unit (clsname, iname, parent)
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

    nextinmsgs = new Message *[0];

    nextoutmsgs = new Message **[omsgidxmask + 1];
    for (uint32_t i = 0; i < omsgidxmask + 1; i++)
        nextoutmsgs[i] = new Message *[1];

    outque_size = new uint32_t[0];

    pbusy_state = 0;
    inmsg_valid_count = 0;
}


/* functions for 'Component' */
bool Module::SetScript (Script *script)
{
    if (this->script)
        DESIGN_WARNING ("overwriting '%s' with '%s'",
                GetFullName().c_str(), this->script->GetName().c_str(), 
                script->GetName().c_str());

    DEBUG_PRINT ("%p", script->GetParent ());
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


/* functions for 'Simulator' */
IssueCount Module::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!GetParent())
    {
        SYSTEM_ERROR ("module '%s' has no parent", GetFullName().c_str());
        icount.error++;
    }

    if (GetDynamicPower() == -1)
    {
        //DESIGN_WARNING ("no dynamic power info", GetFullName().c_str());
        icount.warning++;
    }

    if (GetStaticPower() == -1)
    {
        //DESIGN_WARNING ("no static power info", GetFullName().c_str());
        icount.warning++;
    }

    if (GetClockPeriod() == -1)
    {
        SYSTEM_ERROR ("no clock period (module: %s)", GetFullName().c_str());
        icount.error++;
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
                    DEBUG_PRINT("%s is stalled (capacity %d)",
                            outport.endpt->GetConnectedPortName().c_str(), outport.endpt->GetCapacity());
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
    MICRODEBUG_PRINT ("calc '%s' (stall: %d)", GetFullName().c_str(), stalled);

    Instruction *nextinstr = nullptr;
    if (likely (!stalled))
    {
        if (script)
        {
            operation ("prepare instruction");
            nextinstr = script->NextInstruction ();
        }
    }

    operation ("peak messages from RHS")
    {
        for (auto i = 0; i < ninports; i++)
        {
            if (inports[i].endpt->GetMsgPrototype()->GetType() == Message::PLAIN)
            {
                if (!nextinmsgs[i])
                {
                    if (likely (!stalled))
                    {
                        nextinmsgs[i] = inports[i].endpt->Peek ();
                        DEBUG_PRINT ("peaking message %p", nextinmsgs[i]);
                    }
                }
            }
            else /* TOGGLE */
            {
                Message *inmsg = inports[i].endpt->Peek ();
                if (inmsg)
                {
                    inports[i].endpt->Pop ();
                    if (nextinmsgs[i]) nextinmsgs[i]->Dispose ();
                    nextinmsgs[i] = inmsg;
                }
            }
        }
    }

    if (likely (pdepth == 0 || !stalled))
    {
        operation ("load output que size")
        {
            for (auto i = 0; i < noutports; i++)
                outque_size[i] = outports[i].endpt->GetNumMessages ();
        }

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
                {
                    MarkBusyPipeline ();
                    break;
                }
            }
        }
        
        operation ("pop used messages from RHS")
        {
            for (auto i = 0; i < ninports; i++)
            {
                if (inports[i].endpt->GetMsgPrototype()->GetType() == Message::PLAIN)
                {
                    if (nextinmsgs[i])
                    {
                        inports[i].endpt->Pop ();
                        nextinmsgs[i]->Dispose ();
                        nextinmsgs[i] = nullptr;

                        UpdateInMsgValidCount ();
                        RefreshInMsgValidCount ();
                    }
                }
            }
        }

        CommitPipeline ();
        omsgidx = (omsgidx + 1) & omsgidxmask;
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
                        SYSTEM_ERROR ("attempted to push to full LHS");

                    nextoutmsgs[omsgidx][i] = nullptr;
                }
            }
        }
    }
}


void Module::OnCreatePort (Port &newport)
{
    switch (newport.iotype)
    {
        case PORT_INPUT:
            delete[] nextinmsgs;
            nextinmsgs = new Message *[ninports] ();
            break;
        case PORT_OUTPUT:
            for (uint32_t i = 0; i < omsgidxmask + 1; i++)
            {
                delete[] nextoutmsgs[i];
                nextoutmsgs[i] = new Message *[noutports] ();
            }
            
            delete[] outque_size;
            outque_size = new uint32_t[noutports] ();
            break;
        default:
            SYSTEM_ERROR ("Module cannot have %s type port (module: %s)", 
                    Port::GetTypeString (newport.iotype),
                    GetFullName().c_str());
            break;
    }
}

bool Module::IsValidConnection (Port *port, Endpoint *endpt)
{
    if (port->iotype == Unit::PORT_INPUT)
    {
        if (endpt->GetCapacity () == 0)
        {
            DESIGN_FATAL ("module INUPT endpoint cannot have zero capacity",
                    GetFullName().c_str());
            return false;
        }
    }

    return true;
}
