

class Component: public Interface
{
private:
    struct PortFwdTag
    {
        Interface *unit;
        string portname;
    };

public:
    /* Universal */
    Component (Component *parent, string name);
    Component *GetParent ();

    /* Called by 'Simulator' */
    Interface* GetChild (uint32_t id, PERMIT(Simulator));
    Interface* GetChildByName (string name, PERMIT(Simulator));
    uint32_t GetNumChildren (PERMIT(Simulator));

protected:
    /* Called by parent 'Component' */
    virtual bool Connect (uint32_t portid, Endpoint *endpt) final;

    // Called by this 'Component' */
    bool ExportPort (uint32_t id, string childname, uitn32_t idChildPort);

private:
    Component *parent;

    map<uint32_t, PortFwdTag> mapPorts;
    vector<Interface *> children;
};
