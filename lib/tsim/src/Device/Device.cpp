#include <TSim/Device/Device.h>

#include <TSim/Base/Component.h>
#include <TSim/Base/Unit.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <vector>
#include <string>
#include <map>
#include <set>

using namespace std;


const char *const Device::OUTPORT_NAME = "output";


/* Constructors */
Device::Device (const char *clsname, string iname, Component *parent, Message *msgproto):
    Unit (clsname, iname, parent)
{
    this->msgproto = msgproto;

    pname2port.insert (make_pair (OUTPORT_NAME, &outports[0]));
    noutports++;

    Port &outport = outports[0];
    outport.name = OUTPORT_NAME;
    outport.id = 0;
    outport.iotype = PORT_OUTPUT;
    outport.endpt = nullptr;
    outport.msgproto = msgproto;

    nextinmsgs = new Message *[0];
    nextctrlmsgs = new IntegerMessage[0];
}


string Device::GetClock ()
{
    string clockname = "";

    for (auto i = 0; i < ninports; i++)
    {
        if (!inports[i].endpt) return "";
        
        if (inports[i].endpt->GetParent()->GetClock() == "")
            return "";
        else if (clockname == "")
            clockname = inports[i].endpt->GetParent()->GetClock();
        else if (clockname != inports[i].endpt->GetParent()->GetClock())
            return "";
    }

    return clockname;
}

set<string> Device::GetClockSet ()
{
    set<string> clockset;

    for (auto i = 0; i < ninports; i++)
    {
        set<string> clocks = inports[i].endpt->GetParent()->GetClockSet();
        if (clocks.empty ()) return set<string>();
        
        for (string clock : clocks)
            clockset.insert (clock);
    }

    return clockset;
}

/* functions for 'Component' */
bool Device::IsValidConnection (Port *port, Endpoint *endpt)
{
    // XXX Device-only
    if (port->iotype == Device::PORT_INPUT || port->iotype == Device::PORT_CONTROL)
    {
        if (endpt->GetCapacity () != 0)
        {
            DESIGN_WARNING ("pre-device endpoint must have zero capacity. adjusting..",
                    GetFullName().c_str());
            endpt->SetCapacity (0);
        }
        
        if (endpt->GetParent()->GetLatency() != 0)
        {
            DESIGN_ERROR ("pre-device path must have zero latency",
                    GetFullName().c_str());
            return false;
        }
    }

    // XXX Device-only
    if (port->iotype == Device::PORT_CONTROL)
    {
        if (endpt->GetCapacity () != 0)
        {
            DESIGN_WARNING ("device control endpoint must have zero capacity. adjusting..",
                    GetFullName().c_str());
            endpt->SetCapacity (0);
        }
        
        if (endpt->GetParent()->GetMsgPrototype()->GetType() != Message::TOGGLE)
        {
            DESIGN_ERROR ("control pathway must have TOGGLE message type", GetFullName().c_str());
            return false;
        }
    }

    return true;
}


/* functions for 'Simulator' */
IssueCount Device::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!GetParent())
    {
        SYSTEM_ERROR ("module '%s' has no parent", GetFullName().c_str());
        icount.error++;
    }

    // NOTE: ignore power consumption if not defined
    if (GetDynamicPower() == -1)
    {
        DESIGN_INFO ("no dynamic power info. assuming 0.", GetFullName().c_str());
        SetDynamicPower(0);
    }

    if (GetStaticPower() == -1)
    {
        DESIGN_INFO ("no static power info. assuming 0.", GetFullName().c_str());
        SetStaticPower(0);
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
            DESIGN_ERROR ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
            icount.error++;
        }
    }
    
    for (auto i = 0; i < nctrlports; i++)
    {
        Port &port = ctrlports[i];
        if (!port.endpt)
        {
            DESIGN_ERROR ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
            icount.error++;
        }
    }

    if (!outports[0].endpt)
    {
        DESIGN_ERROR ("disconnected port '%s'", 
                GetFullName().c_str(), outports[0].name.c_str());
        icount.error++;
    }

    return icount;
}


/* Called by 'Device' */
void Device::OnCreatePort (Port &port)
{
    switch (port.iotype)
    {
        case PORT_INPUT:
            delete[] nextinmsgs;
            nextinmsgs = new Message *[ninports] ();
            break;
        case PORT_CONTROL:
            if (port.msgproto != Prototype<IntegerMessage>::Get())
                DESIGN_FATAL ("control input must have IntegerMessage message type",
                        GetName().c_str());

            delete[] nextctrlmsgs;
            nextctrlmsgs = new IntegerMessage[nctrlports] ();
            break;
        default:
            SYSTEM_ERROR ("cannot create %s port (device: %s)",
                    Port::GetTypeString (port.iotype),
                    GetFullName().c_str());
            break;
    }
}
