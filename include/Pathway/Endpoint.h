
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

    /* Called by 'Pathway' */
    virtual bool Assign (Message msg);
    virtual Message Fetch ();

    /* Called by 'Simulator' */
    virtual void Clock (PERMIT(Simulator));

    /* Called by 'Module' */
    bool JoinTo (Module *module, uint32_t idPort, PERMIT(Module));

private:
    const char* clsname;
    Pathway* parent;

    Module *modConn;
    uint32_t idPortConn;
    Register* reg;
};
