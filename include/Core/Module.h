
class Component;

class Module: public Interface
{
private:
    struct Port
    {
        enum IOType { INPUT, OUTPUT, /* TODO INOUT */ };

        // Port Info
        string name;
        IOType iotype;
        Message* msgproto;
        
        // Connected Endpoint
        Endpoint* endpt;
    };

public:
    /* Universal */
    Module (Component *parent, string name);
    Component *GetParent ();

    /* Called by parent 'Component' */
    bool SetScript (Script *script);
    bool SetRegister (Register *reg);
    virtual bool Connect (uint32_t portid, Endpoint *endpt) final;

    /* Called by 'Simulator' */
    virtual bool Validate (PERMIT(Simulator)) final;
    virtual void Clock (PERMIT(Simulator)) final;

protected:  
    /* Called by derived 'Module' */
    bool CreatePort (uint32_t id, Port::IOType iotype, Message* msgproto, string name);

    /* Called by this 'Module' */
    virtual void Operation (Message **msgs, ScriptLine *scrline) = 0;

private:
    Component *parent;

    vector<Port> ports;
    vector<Message *> nextmsgs;

    Script *script;
    Register *reg;
};
