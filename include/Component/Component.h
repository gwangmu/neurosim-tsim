

class Component: public Interface, public IValidatable, public IMetadata
{
private:
    struct PortFwdTag
    {
        Interface *unit;
        string portname;
    };

public:
    /* Universal */
    Component (Component *parent, const char* clsname, string name);
    Component *GetParent ();

    virtual string GetName () final;
    virtual string GetFullName () final;
    virtual const char* GetClassName () final;
    virtual string GetSummary ();

    /* Called by 'Simulator' */
    auto ChildBegin (PERMIT(Simulator));
    auto ChildEnd (PERMIT(Simulator));
    auto ChildNext (auto it, PERMIT(Simulator));

    auto PathwayBegin (PERMIT(Simulator));
    auto PathwayEnd (PERMIT(Simulator));
    auto PathwayNext (auto it, PERMIT(Simulator));

    virtual IssueCount Validate (PERMIT(Simulator)) final;

protected:
    /* Called by parent 'Component' */
    virtual bool Connect (uint32_t portid, Endpoint *endpt) final;

    // Called by this 'Component' */
    bool ExportPort (uint32_t id, string childname, uint32_t idChildPort);

private:
    const char* clsname;
    string name;
    Component *parent;

    map<uint32_t, PortFwdTag> mapPorts;
    vector<Interface *> children;
    vector<Pathway *> pathways;
};
