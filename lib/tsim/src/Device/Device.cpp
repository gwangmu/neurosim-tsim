#include <TSim/Device/Device.h>

#include <TSim/Base/Component.h>
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
Device::Device (const char *clsname, string iname, Component *parent, Message *msgproto):
    Component (clsname, iname, parent)
{
    ninports = nctrlports = 0;
    pname2port.insert (make_pair (GetOutPortName(), &outport));

    this->msgproto = msgproto;

    nextinmsgs = new Message *[0];
    nextctrlmsgs = new IntegerMessage[0];

    clkperiod = -1;
    dynpower = -1;
    stapower = -1;
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


/* functions for 'Component' */
bool Device::Connect (string portname, Endpoint *endpt)
{
    if (!pname2port.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'", GetFullName().c_str(), 
                portname.c_str());
        return false;
    }

    Port *port = pname2port[portname];

    if (port->iotype == Device::PORT_UNKNOWN)
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
            

    if ((port->iotype == Device::PORT_INPUT || port->iotype == Device::PORT_OUTPUT) &&
            !endpt->IsPortCap() && 
            endpt->GetParent()->GetMsgPrototype() != msgproto)
    {
        DESIGN_ERROR ("mismatching message proto %s (of '%s') and %s",
                GetFullName().c_str(), 
                endpt->GetParent()->GetMsgPrototype()->GetClassName (),
                endpt->GetParent()->GetName().c_str(), 
                msgproto->GetClassName ());
        return false;
    }

    if (!(endpt->GetEndpointType() == Endpoint::CAP) &&
            !(port->iotype == Device::PORT_INPUT &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Device::PORT_CONTROL &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Device::PORT_OUTPUT &&
                endpt->GetEndpointType() == Endpoint::LHS))
    {
        DESIGN_ERROR ("incompatible endpoint '%s' to %s port '%s'",
                GetName().c_str(), endpt->GetClassName(),
                Device::Port::GetTypeString(port->iotype), portname.c_str());
        return false;
    }

    port->endpt = endpt;

    if (!endpt->IsPortCap())
        endpt->JoinTo (this, portname);

    return true;
}


double Device::GetConsumedEnergy ()
{
    if (clkperiod == -1)
        SYSTEM_ERROR ("zero module clock period (module: %s)",
                GetFullName().c_str());

    if (dynpower == -1 || stapower == -1)
        return -1;
    else
        return (clkperiod * 10E-9 * 
                (stapower * 10E-9 * (cclass.active + cclass.idle) +
                 dynpower * 10E-9 * cclass.active));
}


Component::CycleClass<double> Device::GetAggregateCycleClass ()
{
    CycleClass<double> cclass_conv;
    cclass_conv.active = cclass.active;
    cclass_conv.idle = cclass.idle;

    return cclass_conv;
}

Component::EventCount<double> Device::GetAggregateEventCount ()
{
    EventCount<double> ecount_conv;
    ecount_conv.stalled = ecount.stalled;

    return ecount_conv;
}

double Device::GetAggregateConsumedEnergy ()
{
    return GetConsumedEnergy ();
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

    if (clkperiod == -1)
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

    if (!outport.endpt)
    {
        DESIGN_ERROR ("disconnected port '%s'", 
                GetFullName().c_str(), outport.name.c_str());
        icount.error++;
    }

    return icount;
}


/* Called by 'Device' */
uint32_t Device::CreatePort (string portname, Device::PortType iotype)
{
    uint32_t id = -1;
    Port port = Port();
    
    port.name = portname;
    port.iotype = iotype;
    port.endpt = nullptr;

    if (iotype == Device::PORT_INPUT)
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
    else if (iotype == Device::PORT_CONTROL)
    {
        if (nctrlports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#rhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = nctrlports++;
        port.id = id;
        ctrlports[id] = port;
        pname2port.insert (make_pair (portname, &ctrlports[id]));

        delete[] nextctrlmsgs;
        nextctrlmsgs = new IntegerMessage[nctrlports] ();
    }
    else
    {
        DESIGN_ERROR ("cannot create %s port", GetFullName().c_str(),
                Device::Port::GetTypeString (iotype));
        return -1;
    }

    return id;
}
