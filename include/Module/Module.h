#pragma once

#include <Component/Interface.h>
#include <Interface/IValidatable.h>
#include <Interface/IClockable.h>
#include <Interface/IMetadata.h>

#include <string>
#include <vector>

using namespace std;

class Component;
class Script;
class Register;
class Endpoint;
class Simulator;
class Message;
class Intruction;


class Module: public Interface, public IValidatable, public IClockable, public IMetadata
{
private:
    struct Port
    {
        enum IOType { UNKNOWN, INPUT, OUTPUT, /* TODO INOUT */ };
        const char* GetIOTypeString (IOType ty)
        {
            static const char *strs[] = { "UNKNOWN", "INPUT", "OUTPUT" };
            return strs[ty];
        }

        // Port Info
        string name;
        uint32_t id;
        IOType iotype;
        Message* msgproto;
        
        // Connected Endpoint
        Endpoint* endpt;
    };

public:
    /* Universal */
    Module (Component *parent, const char* clsname, string name);
    Component *GetParent () { return parent };

    virtual string GetName () final { return name; }
    virtual string GetFullName () final;
    virtual const char* GetClassName () final { return clsname; }
    virtual string GetSummary ();

    /* Called by parent 'Component' */
    Script* GetScript () { return script; }
    Register* GetRegister () { return reg; }
    bool SetScript (Script *script); 
    bool SetRegister (Register *reg);

    virtual bool Connect (string portname, Endpoint *endpt) final;

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

protected:  
    /* Called by derived 'Module' */
    uint32_t CreatePort (string portname, Port::IOType iotype, Message* msgproto);

    /* Called by this 'Module' */
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr) = 0;

private:
    const char* clsname;
    string name;
    Component *parent;

    vector<Port> inports;
    vector<Port> outports;
    map<string, Port *> pname2port;
    
    Message *nextinmsgs[];
    Message *nextoutmsgs[];
    bool outMsgPended;

    Script *script;
    Register *reg;
};
