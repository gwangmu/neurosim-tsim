#pragma once

#include <Base/Metadata.h>
#include <Interface/IValidatable.h>
#include <Interface/IClockable.h>
#include <Base/IssueCount.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Simulator;
class Endpoint;
class Message;


class Pathway: public Metadata, public IValidatable, public IClockable
{
public:
    struct ConnectionAttr
    {
        ConnectionAttr () { latency = 0; bitwidth = 0; }
        uint32_t latency;
        uint32_t bitwidth;
    };

protected:
    class Connection
    {
    public:
        Message* Sample ();
        void Flow ();
        void Assign (Message *newmsg);

    private:
        ConnectionAttr conattr;

        Message *msgprop[];
        uint32_t nprop;
        uint32_t curidx;

        uint32_t PROPIDX_MASK;
    };

public:
    /* Universal */
    Pathway (const char *clsname, Component *parent, 
            ConnectionAttr conattr, Message *msgproto);

    Component* GetParent () { return parent; }
    virtual string GetInstanceName ();
    Message *GetMsgPrototype () { return msgproto; }

    /* Called by 'Component' */
    Endpoint* GetEndpoint (Endpoint::Type type, uint32_t idEndpt = 0);

    ConnectionAttr GetConnectionAttr () { return conn.conattr; }
    bool SetConnectionAttr (ConntectionAttr conattr);

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    virtual uint32_t TargetLHSEndpointID () = 0;

protected:
    bool AddEndpoint (Endpoint::Type type, uint32_t capacity); 

    struct
    {
        vector<EndPoint> lhs;
        vector<EndPoint> rhs;
    } endpts;

private:
    // ready state
    inline void InitReadyState () { rhsreadymask = next_rhsreadymask = (uint32_t)-1 }

    inline bool IsReady (uint32_t rhsid) 
    {
        return (rhsid == (uint32_t)-1 ? 
                rhsreadymask == (uint32_t)-1 : rhsreadymask & (1 << rhsid); 
    }

    inline void SetNextReady (uint32_t rhsid, bool state)
    { 
        next_rhsreadymask ^= (-(uint32_t)state ^ next_rhsreadymask) & (1 << rhsid); 
    }

    inline void UpdateReadyState () { next_rhsreadymask = rhsreadymask; }

    // target LHS
    inline uint32_t GetLHSIDOfThisCycle ()
    { 
        if (stabilize_cycle == 0)
            return tarlhsid;
        else
        {
            stabilize_cycle--;
            return -1;
        }
    }

private:
    Component *parent;

    Connection conn;
    Message *msgproto;

    uint64_t rhsreadymask;
    uint64_t next_rhsreadymask;

    uint32_t lhsid;
    uint32_t stabilize_cycle;
};
