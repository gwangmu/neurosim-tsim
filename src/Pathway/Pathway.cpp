#include <Pathway/Pathway.h>

#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

using namespace std;


Pathway::Connection::Connection ()
{
    msgprop = nullptr;
    nprop = 0;
    curidx = 0;

    PROPIDX_MASK = 0;
}

Message* Pathway::Connection::Sample ()
{
    return msgprop[curidx];
}

void Pathway::Connection::Flow ()
{
    // advance
    msgprop[curidx] = nullptr;
    nextidx = (curidx + 1) & PROPIDX_MASK;
}

void Pathway::Connection::Assign (Message *newmsg)
{
    msgprop[(curidx + conattr.latency) & PROPIDX_MASK] = newmsg;
}
    


Pathway::Pathway (const char *clsname, Component *parent, 
        Pathway::ConnectionAttr conattr, Message *msgproto)
    : Metadata (clsname, "")
{
    if (!parent)
        DESIGN_ERROR ("pathway must have parent", clsname);
    this->parent = parent;
    parent->AddChildPathway (this, KEY(Pathway));

    if (!msgproto)
        DESIGN_ERROR ("pathway with no message prototype (parent: %s)",
                clsname, (parent ? parent->GetFullName().c_str() : "(null)"));
    this->msgproto = msgproto;

    InitReadyState ();

    lhsid = (uint32_t)-1;
    stabilized_cycle = 0;

    SetConnectionAttr (conattr);
}


string Pathway::GetInstanceName ()
{
    string lhs = "(null)";
    string rhs = "(null)";

    for (auto i = 0; i < endpts.lhs.size (); i++)
    {
        if (endpts.lhs[i].GetConnectedModule ())
        {
            lhs = endpts.lhs[i].GetConnectedModule().GetInstanceName().c_str()
                + "." + endpts.lhs[i].GetConnectedPortName().c_str();
            break;
        }
    }

    for (auto i = 0; i < endpts.rhs.size (); i++)
    {
        if (endpts.rhs[i].GetConnectedModule ())
        {
            rhs = endpts.rhs[i].GetConnectedModule().GetInstanceName().c_str()
                + "." + endpts.rhs[i].GetConnectedPortName().c_str();
            break;
        }
    }
    
    return lhs + "-" + rhs;
}


Endpoint* Pathway::GetEndpoint (Endpoint::Type type, uint32_t idEndpt)
{
    vector<Endpoint> *vec = nullptr;
    case (type)
    {
        switch Endpoint::LHS:
            vec = &endpts.lhs;
            break;
        switch Endpoint::RHS:
            vec = &endpts.rhs;
            break;
    }

    if (vec->size () <= idEndpt)
        return nullptr;
    else
        return (*vec)[idLEndpt];
}

bool Pathway::AddEndpoint (string name, Endpoint::Type type, uint32_t capacity) 
{
    if (type == Endpoint::LHS)
    {
        if (endpts.lhs.size () >= 64)
        {
            DESIGN_ERROR ("'#lhs > 64' not allowed", GetName().c_str());
            return false;
        }
        
        endpts.lhs.push_back (Endpoint (name, this, Endpoint::LHS, 
                    capacity, KEY(Pathway)));
    }
    else if (type == Endpoint::RHS)
    {
        if (endpts.rhs.size () >= 64)
        {
            DESIGN_ERROR ("'#rhs > 64' not allowed", GetName().c_str());
            return false;
        }

        endpts.rhs.push_back (Endpoint (name, this, Endpoint::RHS, 
                    capacity, KEY(Pathway)));
    }
    else
    {
        DESIGN_ERROR ("bogus endpoint type '%u'", GetName().c_str(), type);
        return false;
    }

    return true;
}



bool Pathway::SetConnectionAttr (Pathway::ConnectionAttr conattr)
{
    if (conattr.bitwidth == 0)
        DESIGN_WARNING ("zero-bitwidth connection is unrealistic (parent: %s)",
                clsname, parent->GetFullName().c_str());

    if (conattr.latency > ConnectionAttr::CONN_LATENCY_LIMIT)
        DESIGN_FATAL ("connection with %u cycle (> 10000) latency "
                "is not allowed (parent: %s)",
                clsname, conattr.latency, parent->GetFullName().c_str()); 

    conn.conattr = conattr;

    uint32_t propmask = 1;
    for (uint32_t lat = conattr.latency; lat; propmask <<= 1, lat >>= 1);
    propmask -= 1;

    delete msgprop[];
    msgprop = new Message *[propmask + 1]();
    
    conn.PROPIDX_MASK = propmask;
    conn.nprop = 0;
    conn.curidx = 0;

    return true;
}
    


IssueCount Pathway::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (conn.conattr.latency > Pathway::ConnectionAttr::CONN_LATENCY_LIMIT)
    {
        DESIGN_ERROR ("connection latency (%u) exceeded the limit (%u)",
                GetName().c_str(), conn.conattr.latency,
                Pathway::ConnectionAttr::CONN_LATENCY_LIMIT);
        icount.error++;
    }

    if (!msgproto)
    {
        SYSTEM_ERROR ("null message prototype (pathway: %u)", GetName().c_str());
        icount.error++;
    }

    if (sizeof(*msgproto) - sizeof(msgproto->DEST_RHS_ID) < conn.conattr.bitwidth)
    {
        DESIGN_WARNING ("connection bitwidth (%u) smaller than"
                "message prototype size (%u)",
                GetName().c_str(), con.conattr.bitwidth,
                sizeof(*msgproto) - sizeof(msgproto->DEST_RHS_ID));
        icount.warning++;
    }

    for (Endpoint &ept : endpts.lhs)
    {
        if (!ept->GetConnectedModule ())
        {
            DESIGN_ERROR ("disconnected endpoint '%s'",
                    GetName().c_str(), ept.GetName().c_str());
            icount.error++;
        }
    }

    for (Endpoint &ept : endpts.rhs)
    {
        if (!ept->GetConnectedModule ())
        {
            DESIGN_ERROR ("disconnected endpoint '%s'",
                    GetName().c_str(), ept.GetName().c_str());
            icount.error++;
        }
    }

    return icount;
}

void Pathway::PreClock (PERMIT(Simulator))
{
    operation ("push messages (RHS push)")
    {    
        Message *sampledmsg = conn.Sample ();

        if (sampledmsg)
        {
            if (sampledmsg->DEST_RHS_ID == (uint32_t)-1)
            {
                operation ("broadcast message");
                for (auto i = 0; i < endpts.rhs.size(); i++)
                {
                    Endpoint &ept = endpts.rhs[i];
                    if (!ept.Assign (sampledmsg))
                        SIM_WARNING ("message dropped (portname: %s)",
                                GetName().c_str(), ept.GetConnectedPortName().c_str());
                }
            }
            else
            {
                operation ("send message to specific RHS")
                {
                    #ifndef NDEBUG
                    if (sampledmsg->DEST_RHS_ID >= endpts.rhs.size ())
                        SYSTEM_ERROR ("DEST_RHS_ID >= #rhs");
                    #endif

                    Endpoint &ept = endpts.rhs[sampledmsg->DEST_RHS_ID];
                    if (!ept.Assign (sampledmsg))
                        SIM_WARNING ("message dropped (portname: %s)",
                                GetName().c_str(), ept.GetConnectedPortName().c_str());
                }
            }
        }
    }

    operation ("update next ready state");
    for (auto i = 0; i < endpts.rhs.size(); i++)
    {
        SetNextReady (i,
                (uint32_t)(ept.GetConnectedModule().IsStalled ())
                + ept.GetCapacity () - ept.GetNumMessages ()
                < conn.conattr.latency + 1);
    }

    operation ("flow messages in connection");
    conn.Flow ();

    operation ("pop message from LHS (LHS pop)") {
        Message *msg_to_assign = nullptr;
        if (GetLHSIDOfThisCycle () != (uint32_t)-1)
            msg_to_assign = endpts.lhs[GetLHSIDOfThisCycle ()].Peek ();

        if (msg_to_assign && !IsReady (msg_to_assign->DEST_RHS_ID))
            msg_to_assign = nullptr;
        else
            emdpts.lhs[GetLHSIDOfThisCycle ()].Pop ();

        if (msg_to_assign)
            conn.Assign (msg_to_assign);
    }

    operation ("transition ready state");
    UpdateReadyState ();
}

void Pathway::PostClock (PERMIT(Simulator))
{
    uint32_t targetid = TargetLHSEndpointID ();
    if (targetid != lhsid)
    {
        #ifndef NDEBUG
        if (targetid >= endpts.lhs.size ())
        {
            SIM_FATAL ("target LHS ID (%u) is out of bound (0 to %u)",
                    GetName().c_str(), targetid, endpts.lhs.size () - 1);
        }
        #endif

        if (stabilize_time == 0)
            stabilize_time = conn.conattr.latency;
        lhsid = targetid;
    }
}

