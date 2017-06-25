#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Interface/IValidatable.h>
#include <TSim/Interface/IClockable.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Base/IssueCount.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Simulator;
class Message;


class Pathway: public Metadata, public IValidatable, public IClockable
{
public:
    struct ConnectionAttr
    {
        ConnectionAttr () { latency = 0; bitwidth = 0; }

        static const uint32_t CONN_LATENCY_LIMIT = 10000;

        uint32_t latency;
        uint32_t bitwidth;
    };

protected:
    class Connection
    {
    public:
        Connection ();

        Message* Sample ();
        void Flow ();
        void Assign (Message *newmsg);

    //private:      // FIXME for the sake of convenience XD
        ConnectionAttr conattr;

        Message **msgprop;
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
    bool SetConnectionAttr (ConnectionAttr conattr);

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    virtual uint32_t TargetLHSEndpointID () = 0;

protected:
    bool AddEndpoint (string name, Endpoint::Type type, uint32_t capacity); 

    auto BeginLHSEndpoint () { return endpts.lhs.begin (); }
    auto EndLHSEndpoint () { return endpts.lhs.end (); }
    auto BeginRHSEndpoint () { return endpts.rhs.begin (); }
    auto EndRHSEndpoint () { return endpts.rhs.end (); }

private:
    // ready state
    inline void InitReadyState ();
    inline bool IsReady (uint32_t rhsid);
    inline void SetNextReady (uint32_t rhsid, bool state);
    inline void UpdateReadyState ();

    // target LHS
    inline uint32_t GetLHSIDOfThisCycle ();

private:
    Component *parent;

    Connection conn;
    Message *msgproto;

    struct
    {
        vector<Endpoint> lhs;
        vector<Endpoint> rhs;
    } endpts;

    uint64_t rhsreadymask;
    uint64_t next_rhsreadymask;

    uint32_t lhsid;
    uint32_t stabilize_cycle;
};


/* Inline functions */
inline void Pathway::InitReadyState () 
{
    rhsreadymask = next_rhsreadymask = (uint32_t)-1;
}

inline bool Pathway::IsReady (uint32_t rhsid) 
{
    return (rhsid == (uint32_t)-1 ? 
            rhsreadymask == (uint32_t)-1 : rhsreadymask & (1 << rhsid));
}

inline void Pathway::SetNextReady (uint32_t rhsid, bool state)
{ 
    next_rhsreadymask ^= (-(uint32_t)state ^ next_rhsreadymask) & (1 << rhsid); 
}

inline void Pathway::UpdateReadyState () 
{
    next_rhsreadymask = rhsreadymask;
}


inline uint32_t Pathway::GetLHSIDOfThisCycle ()
{ 
    if (stabilize_cycle == 0)
        return lhsid;
    else
    {
        stabilize_cycle--;
        return -1;
    }
}
