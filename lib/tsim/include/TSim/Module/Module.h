#pragma once

#include <TSim/Base/Component.h>
#include <TSim/Interface/IClockable.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Script;
class Register;
class Endpoint;
class Simulator;
class Message;
class Instruction;


class Module: public Component, public IClockable
{
public:
    enum PortType { PORT_UNKNOWN, PORT_INPUT, PORT_OUTPUT };

private:
    struct Port
    {
        static const char* GetTypeString (PortType ty)
        {
            static const char *strs[] = { "UNKNOWN", "INPUT", "OUTPUT" };
            return strs[ty];
        }

        Port () {};

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
    Module (const char* clsname, string iname, Component *parent, uint32_t pdepth);

    Script* GetScript () { return script; }
    Register* GetRegister () { return reg; }
    virtual Module* GetModule (string name);
    virtual uint32_t GetNumChildModules () final { return 1; }

    void SetClockPeriod (uint32_t period, PERMIT(Simulator)) { clkperiod = period; }

    void SetDynamicPower (uint32_t pow, PERMIT(Simulator)) { dynpower = pow; }
    void SetStaticPower (uint32_t pow, PERMIT(Simulator)) { stapower = pow; }
    uint32_t GetDynamicPower () { return dynpower; }
    uint32_t GetStaticPower () { return stapower; }
    double GetConsumedEnergy ();

    virtual CycleClass<double> GetAggregateCycleClass ();
    virtual EventCount<double> GetAggregateEventCount ();
    virtual double GetAggregateConsumedEnergy ();
    
    const CycleClass<uint64_t>& GetCycleClass () { return cclass; }
    const EventCount<uint64_t>& GetEventCount () { return ecount; }

    uint32_t GetNumInPorts () { return ninports; }
    uint32_t GetNumOutPorts () { return noutports; }
    string GetInPortName (uint32_t idx) { return inports[idx].name; }
    string GetOutPortName (uint32_t idx) { return outports[idx].name; }
    Endpoint* GetInEndpoint (uint32_t idx) { return inports[idx].endpt; }
    Endpoint* GetOutEndpoint (uint32_t idx) { return outports[idx].endpt; }

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator));
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    /* Called by parent 'Component' */
    bool SetScript (Script *script); 
    bool SetRegister (Register *reg);

    virtual bool Connect (string portname, Endpoint *endpt) final;

protected:  
    /* Called by derived 'Module' */
    uint32_t CreatePort (string portname, PortType iotype, Message* msgproto);

    /* Called by this 'Module' */
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr) = 0;

    /*>>> !SEALED! <<<*/
    virtual bool ExportPort (string, Component *, string) { PROHIBITED; return false; }
    virtual bool AddChild (Component *) { PROHIBITED; return false;}

private:
    static const uint32_t MAX_PDEPTH = 64;

    // property
    uint32_t clkperiod;
    uint32_t dynpower;
    uint32_t stapower;

    // port management
    Port inports[MAX_MODULE_PORTS];
    Port outports[MAX_MODULE_PORTS];
    uint32_t ninports;
    uint32_t noutports;
    uint32_t *outque_size;
    map<string, Port *> pname2port;
    
    // pipeline management
    inline void MarkBusyPipeline () { pbusy_state |= (1 << ((omsgidx + pdepth) & omsgidxmask)); }
    inline void CommitPipeline () { pbusy_state &= ~(1 << omsgidx); }
    inline bool IsIdle () { return pbusy_state == 0; }

    Message **nextinmsgs;
    Message ***nextoutmsgs;
    uint32_t omsgidx;
    uint32_t omsgidxmask;

    uint32_t pdepth;
    uint64_t pbusy_state;

    // module state
    bool stalled;
    
    // add-ons
    Script *script;
    Register *reg;

    // report
    CycleClass<uint64_t> cclass;
    EventCount<uint64_t> ecount;
};
