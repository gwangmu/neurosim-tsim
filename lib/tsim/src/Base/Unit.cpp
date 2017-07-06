#include <TSim/Base/Unit.h>

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
Unit::Unit (const char *clsname, string iname, Component *parent):
    Component (clsname, iname, parent)
{
    ninports = noutports = nctrlports = 0;

    clkperiod = -1;
    dynpower = -1;
    stapower = -1;
}


bool Unit::Connect (string portname, Endpoint *endpt)
{
    if (!pname2port.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'", GetFullName().c_str(), 
                portname.c_str());
        return false;
    }

    Port *port = pname2port[portname];

    if (port->iotype == Unit::PORT_UNKNOWN)
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
            !(port->iotype == Unit::PORT_INPUT &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Unit::PORT_CONTROL &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Unit::PORT_OUTPUT &&
                endpt->GetEndpointType() == Endpoint::LHS))
    {
        DESIGN_ERROR ("incompatible endpoint '%s' to %s port '%s'",
                GetName().c_str(), endpt->GetClassName(),
                Unit::Port::GetTypeString(port->iotype), portname.c_str());
        return false;
    }

    if (!IsValidConnection (port, endpt))
        return false;

    port->endpt = endpt;
    if (!endpt->IsPortCap())
        endpt->JoinTo (this, portname);

    return true;
}


Unit* Unit::GetUnit (string name)
{
    if (GetInstanceName() == name)
        return this;
    else
        return nullptr;
}


double Unit::GetConsumedEnergy ()
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


Component::CycleClass<double> Unit::GetAggregateCycleClass ()
{
    CycleClass<double> cclass_conv;
    cclass_conv.active = cclass.active;
    cclass_conv.idle = cclass.idle;

    return cclass_conv;
}

Component::EventCount<double> Unit::GetAggregateEventCount ()
{
    EventCount<double> ecount_conv;
    ecount_conv.stalled = ecount.stalled;
    // NOTE: ecount.oport_full is non-aggregatable

    return ecount_conv;
}

double Unit::GetAggregateConsumedEnergy ()
{
    return GetConsumedEnergy ();
}


bool Unit::IsInputPort (string portname)
{
    if (!pname2port.count (portname))
        return false;

    return (pname2port[portname]->iotype == PORT_INPUT);
}

bool Unit::IsOutputPort (string portname)
{
    if (!pname2port.count (portname))
        return false;

    return (pname2port[portname]->iotype == PORT_OUTPUT);
}

bool Unit::IsControlPort (string portname)
{
    if (!pname2port.count (portname))
        return false;

    return (pname2port[portname]->iotype == PORT_CONTROL);
}


string Unit::GetGraphVizBody (uint32_t level)
{
    string body = "";

#define ADDLINE(str) body += (string(level, '\t') + str + string("\n"));

    string inputs = "";
    for (auto i = 0; i < ninports; i++)
    {
        inputs += string("<") + inports[i].name + ">" + inports[i].name;
        if (i < ninports + nctrlports - 1) inputs += "|";
    }

    for (auto i = 0; i < nctrlports; i++)
    {
        inputs += string("<") + ctrlports[i].name + ">" + ctrlports[i].name;
        if (i < nctrlports - 1) inputs += "|";
    }

    string outputs = "";
    for (auto i = 0; i < noutports; i++)
    {
        outputs += string("<") + outports[i].name + ">" + outports[i].name;
        if (i < noutports - 1) outputs += "|";
    }

    ADDLINE (GetInstanceName() + string(" [label=\"{ {") +
            inputs + "}|" + string(GetClassName()) + "\\n" + GetInstanceName() + "\\n" + 
            "(clock: " + GetClock() + ")|{" +
            outputs + "} }\"];");

#undef ADDLINE

    return body;
}


/* Called by 'Unit' */
uint32_t Unit::CreatePort (string portname, Unit::PortType iotype,
        Message* msgproto)
{
    uint32_t id = -1;
    Port port = Port();
    
    port.name = portname;
    port.iotype = iotype;
    port.msgproto = msgproto;
    port.endpt = nullptr;

    if (iotype == Unit::PORT_INPUT)
    {
        if (ninports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#lhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = ninports++;
        port.id = id;
        inports[id] = port;
        pname2port.insert (make_pair (portname, &inports[id]));

        OnCreatePort (inports[id]);
    }
    else if (iotype == Unit::PORT_OUTPUT)
    {
        if (noutports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#rhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = noutports++;
        port.id = id;
        outports[id] = port;
        pname2port.insert (make_pair (portname, &outports[id]));

        OnCreatePort (outports[id]);
    }
    else if (iotype == Unit::PORT_CONTROL)
    {
        if (nctrlports > MAX_MODULE_PORTS)
            DESIGN_FATAL ("#rhs > %u not allowed", GetName().c_str(), MAX_MODULE_PORTS);

        id = nctrlports++;
        port.id = id;
        ctrlports[id] = port;
        pname2port.insert (make_pair (portname, &ctrlports[id]));

        OnCreatePort (ctrlports[id]);
    }
    else
    {
        DESIGN_ERROR ("cannot create %s port", GetFullName().c_str(),
                Unit::Port::GetTypeString (iotype));
        return -1;
    }

    return id;
}
