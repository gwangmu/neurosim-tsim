#pragma once

#include <TSim/Base/Component.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Interface/IClockable.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Module;
class Endpoint;
class Simulator;
class Message;


class Device: public Component, public IClockable
{
public:
    enum PortType { PORT_UNKNOWN, PORT_CONTROL, PORT_INPUT, PORT_OUTPUT };

protected:
    struct Port
    {
        static const char* GetTypeString (PortType ty)
        {
            static const char *strs[] = { "UNKNOWN", "CONTROL", "INPUT" };
            return strs[ty];
        }

        Port () {};

        // Port Info
        string name;
        uint32_t id;
        PortType iotype;
        
        // Connected Endpoint
        Endpoint* endpt;
    };

public:
    /* Universal */
    Device (const char* clsname, string iname, Component *parent, Message *msgproto);

    void SetClockPeriod (uint32_t period, PERMIT(Simulator)) { clkperiod = period; }

    void SetDynamicPower (uint32_t pow, PERMIT(Simulator)) { dynpower = pow; }
    void SetStaticPower (uint32_t pow, PERMIT(Simulator)) { stapower = pow; }
    uint32_t GetDynamicPower () { return dynpower; }
    uint32_t GetStaticPower () { return stapower; }
    double GetConsumedEnergy ();

    virtual Module* GetModule (string name) { return nullptr; }

    virtual CycleClass<double> GetAggregateCycleClass ();
    virtual EventCount<double> GetAggregateEventCount ();
    virtual double GetAggregateConsumedEnergy ();
    
    const CycleClass<uint64_t>& GetCycleClass () { return cclass; }
    const EventCount<uint64_t>& GetEventCount () { return ecount; }

    uint32_t GetNumInPorts () { return ninports; }
    uint32_t GetNumCtrlPorts () { return nctrlports; }
    uint32_t GetNumOutPorts () { return 1; }
    string GetInPortName (uint32_t idx) { return inports[idx].name; }
    string GetCtrlPortName (uint32_t idx) { return ctrlports[idx].name; }
    string GetOutPortName () { return "output"; }
    Endpoint* GetInEndpoint (uint32_t idx) { return inports[idx].endpt; }
    Endpoint* GetCtrlEndpoint (uint32_t idx) { return ctrlports[idx].endpt; }
    Endpoint* GetOutEndpoint () { return outport.endpt; }

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator));
    virtual void PreClock (PERMIT(Simulator)) = 0;
    virtual void PostClock (PERMIT(Simulator)) = 0;

    virtual bool Connect (string portname, Endpoint *endpt) final;

protected:  
    /* Called by derived 'Module' */
    uint32_t CreatePort (string portname, PortType iotype);

    /*>>> !SEALED! <<<*/
    virtual bool ExportPort (string, Component *, string) { PROHIBITED; return false; }
    virtual bool AddChild (Component *) { PROHIBITED; return false;}

    // TODO: need to polish. put these into private
    // port management
    Port inports[MAX_MODULE_PORTS];
    Port ctrlports[MAX_MODULE_PORTS];
    Port outport;
    uint32_t ninports;
    uint32_t nctrlports;
    
    Message **nextinmsgs;
    IntegerMessage *nextctrlmsgs;

    // report
    CycleClass<uint64_t> cclass;
    EventCount<uint64_t> ecount;

private:
    // property
    uint32_t clkperiod;
    uint32_t dynpower;
    uint32_t stapower;

    // port management
    Message* msgproto;
    map<string, Port *> pname2port;
};
