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
private:
    static const uint32_t MAX_ENDPOINTS = 64;

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

    template <typename T>
    struct CycleClass
    {
        T propagating = 0;
        T idle = 0;
    };

    template <typename T>
    struct EventCount
    {
        T rhs_blocked[MAX_ENDPOINTS] = {0};
        T msgdrop = 0;
    };

protected:
    class Connection
    {
    public:
        Connection (Pathway *parent);

        Message* Sample ();
        void Flow ();
        void Assign (Message *newmsg);
        void NullifyNow ();

    //private:      // FIXME for the sake of convenience XD
        Pathway *const parent;
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

    void SetClockPeriod (uint32_t period, PERMIT(Simulator)) { clkperiod = period; }

    void SetDissipationPower (uint32_t pow, PERMIT(Simulator)) { dispower = pow; }
    uint32_t GetDissipationPower () { return dispower; }
    double GetConsumedEnergy ();

    uint32_t GetLatency () { return conn.conattr.latency; }
    uint32_t GetBitWidth () { return conn.conattr.bitwidth; }

    Endpoint& GetLHS (uint32_t idx) 
    { 
        if (unlikely (idx > endpts.lhs.size ()))
            SIM_ERROR ("attempted to access out-of-bound endpoint", GetName().c_str());

        return endpts.lhs[idx]; 
    }

    Endpoint& GetRHS (uint32_t idx)
    { 
        if (unlikely (idx > endpts.rhs.size ()))
            SIM_ERROR ("attempted to access out-of-bound endpoint", GetName().c_str());

        return endpts.rhs[idx]; 
    }

    uint32_t GetNumLHS () { return endpts.lhs.size (); }
    uint32_t GetNumRHS () { return endpts.rhs.size (); }

    bool IsPostDevicePathway ();
    bool IsControlPathway ();

    const CycleClass<uint64_t>& GetCycleClass () { return cclass; }
    const EventCount<uint64_t>& GetEventCount () { return ecount; }

    /* Called by 'Component' */
    Endpoint* GetEndpoint (Endpoint::Type type, uint32_t idEndpt = 0);

    ConnectionAttr GetConnectionAttr () { return conn.conattr; }
    bool SetConnectionAttr (ConnectionAttr conattr);

    /* Called by following 'Mux', through child 'Endpoint' */
    void SetExtReadyState (uint32_t rhsid, bool state)
    {
        rhsreadymask_ext ^= (-(uint32_t)state ^ rhsreadymask_ext) & (1 << rhsid); 
    }

    /* Called by preceding 'Mux', through child 'Endpoint' */
    bool IsBroadcastBlocked (uint32_t lhsid)
    {
        return GetTargetLHSID() != lhsid || !IsReady(-1);
    }

    /* Called by 'Simulator' */
    virtual IssueCount Validate (PERMIT(Simulator)) final;
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;

    virtual uint32_t NextTargetLHSEndpointID () = 0;

protected:
    /* Called by derived 'Pathway' */
    bool AddEndpoint (string name, Endpoint::Type type, uint32_t capacity); 
    bool IsStabilizing () { return stabilize_cycle > 0; }

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
    const uint32_t INITIAL_LHSID = 0;

    Component *parent;

    // property
    uint32_t clkperiod;
    uint32_t dispower;

    // connection
    Connection conn;
    Message *msgproto;

    // endpoints
    struct
    {
        vector<Endpoint> lhs;
        vector<Endpoint> rhs;
    } endpts;

    uint64_t rhsreadymask;
    uint64_t rhsreadymask_ext;
    uint64_t next_rhsreadymask;

    uint32_t lhsid;
    uint32_t stabilize_cycle;

    // report
    CycleClass<uint64_t> cclass;
    EventCount<uint64_t> ecount;
};


/* Inline functions */
// FIXME need to be polished
inline void Pathway::InitReadyState () 
{
    rhsreadymask = next_rhsreadymask = rhsreadymask_ext = (uint32_t)-1;
}

inline bool Pathway::IsReady (uint32_t rhsid) 
{
    return (rhsid == (uint32_t)-1 ? 
            (rhsreadymask & rhsreadymask_ext) == (uint32_t)-1 :
            (rhsreadymask & rhsreadymask_ext) & (1 << rhsid));
}

inline void Pathway::SetNextReady (uint32_t rhsid, bool state)
{ 
    next_rhsreadymask ^= (-(uint32_t)state ^ next_rhsreadymask) & (1 << rhsid); 
}

inline void Pathway::UpdateReadyState () 
{
    rhsreadymask = next_rhsreadymask;
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
