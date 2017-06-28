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

        Port ();

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
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr) = 0;

    /*>>> !SEALED! <<<*/
    virtual bool ExportPort (string, Component *, string) { PROHIBITED; return false; }
    virtual bool AddChild (Component *) { PROHIBITED; return false;}

private:
    vector<Port> inports;
    vector<Port> outports;
    map<string, Port *> pname2port;
    
    uint32_t pdepth;
    Message **nextinmsgs;
    Message ***nextoutmsgs;
    uint32_t omsgidx;
    uint32_t omsgidxmask;

    bool stalled;

    Script *script;
    Register *reg;
};
