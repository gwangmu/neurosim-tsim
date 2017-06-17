#include <inttypes.h>

class Unit;

struct ConnectionAttr
{
    uint32_t latency;
    uint32_t bitwidth;
};

class Pathway: public IClockable, public IValidatable, public IMetadata
{
protected:
    // Inner Class
    class Connection
    {
    public:
        // Operations
        void Switch (Message msg);
        Message Sample ();

        // ConnectionAttr Interfaces
        ConnectionAttr GetConnectionAttr ();
        bool SetConnectionAttr (ConntectionAttr conattr);

    private:
        ConnectionAttr conattr;

        Message msgCurrent;
        vector<Message> vecPendingMsgs;
        uint32_t idxNextMsg;
    };

public:
    /* Universal */
    Pathway (ConnectionAttr conattr, Message *msgproto, const char* clsname);

    virtual string GetName () final { return "(noname)"; }
    virtual string GetFullName () final;
    virtual const char* GetClassName () final;
    virtual string GetSummary ();

    /* Called by 'Component' */
    Message *GetMsgPrototype ();

    LHSEndpoint* GetLHSEndpt (uint32_t idLEndpt = 0);
    RHSEndpoint* GetRHSEndpt (uint32_t idREndpt = 0);
    // UniEndpoint* GetUniEndpt (uint32_t idUEndpt = 0); TODO

    ConnectionAttr GetConnectionAttr ();
    bool SetConnectionAttr (ConntectionAttr conattr, PERMIT(Component));

    Component* GetParent ();
    bool SetParent (Component *parent, PERMIT(Component));

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) = 0;
    virtual void Clock (PERMIT(Simulator)) = 0;

protected:
    struct
    {
        vector<LHSEndPoint> lhs;
        vector<RHSEndPoint> rhs;
    } endpts;

private:
    const char* clsname;
    Component *parent;

    Connection conn;
    Message *msgproto;
};
