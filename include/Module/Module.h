
class Component;

class Module: public Interface, public IValidatable, public IClockable, public IMetadata
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
    Module (Component *parent, const char* clsname, string name);
    Component *GetParent ();

    virtual string GetName () final;
    virtual string GetFullName () final;
    virtual const char* GetClassName () final;
    virtual string GetSummary ();

    /* Called by parent 'Component' */
    Script* GetScript ();
    Register* GetRegister ();
    bool SetScript (Script *script);
    bool SetRegister (Register *reg);

    virtual bool Connect (uint32_t portid, Endpoint *endpt) final;

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void Clock (PERMIT(Simulator)) final;

protected:  
    /* Called by derived 'Module' */
    bool CreatePort (uint32_t id, Port::IOType iotype, Message* msgproto, string name);

    /* Called by this 'Module' */
    virtual void Operation (vector<Message *> msgs, Instruction *instr) = 0;

private:
    const char* clsname;
    string name;
    Component *parent;

    vector<Port> ports;
    vector<Message *> nextmsgs;

    Script *script;
    Register *reg;
};
