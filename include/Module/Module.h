#pragma once

#include <Base/Component.h>
#include <Interface/IClockable.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Script;
class Register;
class Endpoint;
class Simulator;
class Message;
class Intruction;


class Module: public Component, public IClockable
{
private:
    struct Port
    {
        enum Type { UNKNOWN, INPUT, OUTPUT, /* TODO INOUT */ };
        static const char* GetTypeString (Type ty)
        {
            static const char *strs[] = { "UNKNOWN", "INPUT", "OUTPUT" };
            return strs[ty];
        }

        // Port Info
        string name;
        uint32_t id;
        Type iotype;
        Message* msgproto;
        
        // Connected Endpoint
        Endpoint* endpt;
    };

public:
    /* Universal */
    Module (const char* clsname, string iname, Component *parent);

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    bool IsStalled () { return outMsgPended; }

protected:  
    /* Called by parent 'Component' */
    Script* GetScript () { return script; }
    Register* GetRegister () { return reg; }
    bool SetScript (Script *script); 
    bool SetRegister (Register *reg);

    virtual bool Connect (string portname, Endpoint *endpt) final;

    /* Called by derived 'Module' */
    uint32_t CreatePort (string portname, Port::Type iotype, Message* msgproto);

    /* Called by this 'Module' */
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr) = 0;

    /*>>> !SEALED! <<<*/
    virtual bool ExportPort (string, Component *, string) { PROHIBITED; }
    virtual bool AddChild (Component *) { PROHIBITED; }

private:
    vector<Port> inports;
    vector<Port> outports;
    map<string, Port *> pname2port;
    
    Message *nextinmsgs[];
    Message *nextoutmsgs[];
    bool outMsgPended;

    Script *script;
    Register *reg;
};
