#include <inttypes.h>

class Unit;

struct ConnectionAttr
{
    uint32_t latency;
    uint32_t bitwidth;
};

class Pathway: public IClockable
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
    Pathway (ConnectionAttr conattr, Message *msgproto);

    // Endpoint Interfaces
    LHSEndpoint* LHSEndpt (uint32_t idLEndpt = 0);
    RHSEndpoint* RHSEndpt (uint32_t idREndpt = 0);
    // UniEndpoint* UniEndpt (uint32_t idUEndpt = 0); TODO

    // ConnectionAttr Interfaces
    ConnectionAttr GetConnectionAttr ();
    bool SetConnectionAttr (ConntectionAttr conattr);

    // Simulation
    virtual void Clock (PERMIT(Simulator)) = 0;

    // Properties
    Message *MsgPrototype ();

protected:
    struct
    {
        vector<LHSEndPoint> lhs;
        vector<RHSEndPoint> rhs;
    } endpts;

private:
    Connection conn;
    Message *msgproto;
};
