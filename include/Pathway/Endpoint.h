
class Pathway;

class Endpoint: public IClockable, public IMetadata
{
public:
    Endpoint (Pathway *parent, const char* clsname, PERMIT(Pathway));

    /* Universal */
    virtual string GetName () final { return "(noname)"; }
    virtual string GetFullName () final { return "(noname)"; }
    virtual const char* GetClassName () final;
    virtual string GetSummary ();

    Pathway* GetParent ();
    Module* GetConnectedModule ();
    uint32_t GetConnectedPortID (); 

    /* Called by 'Pathway' and 'Module' */
    // FIXME enforcing LHS, RHS caller classes?
    virtual bool IsAssignable ();
    virtual bool Assign (Message *msg);
    virtual Message* Fetch ();

    /* Called by 'Simulator' */
    virtual void PreClock (PERMIT(Simulator));
    virtual void PostClock (PERMIT(Simulator));

    /* Called by 'Module' */
    bool JoinTo (Module *module, uint32_t idPort, PERMIT(Module));

protected:
    virtual uint32_t GetCapacity () = 0;
    virtual uint32_t GetSize () = 0;
    virtual bool IsFull () = 0;
    virtual bool IsEmpty () = 0;

private:
    const char* clsname;
    Pathway* parent;

    Module *modConn;
    uint32_t idPortConn;
    // Register *reg;     // TODO for future use to estimate RAM size
};
