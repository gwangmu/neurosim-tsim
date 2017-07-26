#pragma once

#include <TSim/Base/Component.h>
#include <TSim/Interface/IClockable.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Endpoint;
class Simulator;
class Message;


class Unit: public Component, public IClockable
{
public:
    enum PortType { PORT_UNKNOWN, PORT_INPUT, PORT_CONTROL, PORT_OUTPUT };

protected:
    struct Port
    {
        static const char* GetTypeString (PortType ty)
        {
            static const char *strs[] = { "UNKNOWN", "INPUT", "OUTPUT" };
            return strs[ty];
        }

        Port () 
            : name (""), id (-1), iotype (PORT_UNKNOWN), 
            msgproto (nullptr), endpt (nullptr) {};

        // Port Info
        string name;
        uint32_t id;
        PortType iotype;
        Message* msgproto;
        
        // Connected Endpoint
        Endpoint* endpt;
    };

public:
    /* Universal */
    Unit (const char* clsname, string iname, Component *parent);

    virtual Component* GetComponent (string name) { return nullptr; }
    virtual Unit* GetUnit (string name);
    virtual uint32_t GetNumChildModules () final { return 1; }

    uint32_t GetClockPeriod () { return clkperiod; }
    void SetClockPeriod (uint32_t period, PERMIT(Simulator)) { clkperiod = period; }

    void SetDynamicPower (uint32_t pow, PERMIT(Simulator)) { dynpower = pow; }
    void SetStaticPower (uint32_t pow, PERMIT(Simulator)) { stapower = pow; }
    uint32_t GetDynamicPower () { return dynpower; }
    uint32_t GetStaticPower () { return stapower; }
    virtual double GetConsumedEnergy ();

    uint32_t GetTotalCycleCount () { return cclass.active + cclass.idle; }

    virtual CycleClass<double> GetAggregateCycleClass ();
    virtual EventCount<double> GetAggregateEventCount ();
    virtual double GetAggregateConsumedEnergy ();
    
    const CycleClass<uint64_t>& GetCycleClass () { return cclass; }
    const EventCount<uint64_t>& GetEventCount () { return ecount; }

    uint32_t GetNumInPorts () { return ninports; }
    uint32_t GetNumOutPorts () { return noutports; }
    uint32_t GetNumCtrlPorts () { return nctrlports; }
    uint32_t GetInPortID (string pname) { return (pname2port.count(pname) ? pname2port[pname]->id : -1); }  // TODO port type check
    uint32_t GetOutPortID (string pname) { return (pname2port.count(pname) ? pname2port[pname]->id : -1); }
    uint32_t GetCtrlPortID (string pname) { return (pname2port.count(pname) ? pname2port[pname]->id : -1); }
    string GetInPortName (uint32_t idx) { return inports[idx].name; }
    string GetOutPortName (uint32_t idx) { return outports[idx].name; }
    string GetCtrlPortName (uint32_t idx) { return ctrlports[idx].name; }
    Endpoint* GetInEndpoint (uint32_t idx) { return inports[idx].endpt; }
    Endpoint* GetOutEndpoint (uint32_t idx) { return outports[idx].endpt; }
    Endpoint* GetCtrlEndpoint (uint32_t idx) { return ctrlports[idx].endpt; }

    bool IsInputPort (string portname);
    bool IsOutputPort (string portname);
    bool IsControlPort (string portname);

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) = 0;
    virtual void PreClock (PERMIT(Simulator)) = 0;
    virtual void PostClock (PERMIT(Simulator)) = 0;

    virtual string GetGraphVizBody (uint32_t level) final;

    /* Called by parent 'Component' */
    virtual bool Connect (string portname, Endpoint *endpt) final;
    virtual bool IsValidConnection (Port *port, Endpoint *endpt) { return true; }

protected:  
    /* Called by derived 'Unit' */
    uint32_t CreatePort (string portname, PortType iotype, Message* msgproto);
    virtual void OnCreatePort (Port &newport) = 0;

    /*>>> !SEALED! <<<*/
    virtual bool ExportPort (string, Component *, string) final 
        { PROHIBITED; return false; }
    virtual bool AddChild (Component *) final { PROHIBITED; return false;}

// TODO: put these in the private zone.
protected:
    // port management
    Port inports[MAX_MODULE_PORTS];
    Port outports[MAX_MODULE_PORTS];
    Port ctrlports[MAX_MODULE_PORTS];
    uint32_t ninports;
    uint32_t noutports;
    uint32_t nctrlports;
    map<string, Port *> pname2port;
    
    // report
    CycleClass<uint64_t> cclass;
    EventCount<uint64_t> ecount;

private:
    // property
    uint32_t clkperiod;
    uint32_t dynpower;
    uint32_t stapower;
};
