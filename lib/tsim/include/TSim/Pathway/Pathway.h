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
        ConnectionAttr (uint32_t latency, uint32_t bitwidth)
        {
            this->latency = latency;
            this->bitwidth = bitwidth;
        }

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
    string GetClock ();

    /* Called by 'Component' */
    Endpoint* GetEndpoint (Endpoint::Type type, uint32_t idEndpt = 0);

    ConnectionAttr GetConnectionAttr () { return conn.conattr; }
    bool SetConnectionAttr (ConnectionAttr conattr);

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    virtual uint32_t NextTargetLHSEndpointID () = 0;

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
    inline void SetNewTargetLHSID (uint32_t lhsid);
    inline uint32_t GetTargetLHSID ();
    inline void UpdateStabilizeCycle ();

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
// FIXME need to be polished
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


inline void Pathway::SetNewTargetLHSID (uint32_t lhsid)
{
    if (this->lhsid == lhsid) return;

    if (this->lhsid != -1)
        endpts.lhs[this->lhsid].SetSelectedLHS (false);

    if (this->lhsid != -1 || stabilize_cycle == 0)
        stabilize_cycle = conn.conattr.latency;

    if (stabilize_cycle == 0 && lhsid != -1)
        endpts.lhs[lhsid].SetSelectedLHS (true);

    this->lhsid = lhsid;
}

inline uint32_t Pathway::GetTargetLHSID ()
{ 
    if (stabilize_cycle == 0)
        return lhsid;
    else
        return -1;
}

inline void Pathway::UpdateStabilizeCycle ()
{
    if (stabilize_cycle != 0)
        stabilize_cycle--;

    if (stabilize_cycle == 0 && lhsid != -1)
        endpts.lhs[lhsid].SetSelectedLHS (true);
}
