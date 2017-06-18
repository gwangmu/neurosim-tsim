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

void Pathway::Connection::Flow (Message *newmsg)
{
    // advance
    msgprop[curidx] = nullptr;
    nextidx = (curidx + 1) & PROPIDX_MASK;

    // add new message
    if (newmsg)
        msgprop[(curidx + conattr.latency) & PROPIDX_MASK] = newmsg;
}
    


Pathway::Pathway (const char *clsname, Component *parent, 
        Pathway::ConnectionAttr conattr, Message *msgproto)
    : Metadata (clsname, "")
{
    if (!parent)
        DESIGN_WARNING ("pathway with no parent", clsname);
    this->parent = parent;

    if (!msgproto)
        DESIGN_ERROR ("pathway with no message prototype (parent: %s)",
                clsname, parent->GetFullName().c_str());
    this->msgproto = msgproto;

    ready = true;
    next_ready = true;

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

bool Pathway::AddEndpoint (Endpoint::Type type, uint32_t capacity) 
{
    if (type == Endpoint::LHS)
    {
        if (endpts.lhs.size () == 1)
        {
            SYSTEM_ERROR ("'#lhs > 1' not supported");
            return false;
        }
        
        endpts.lhs.push_back (Endpoint (this, Endpoint::LHS, capacity, KEY(Pathway)));
    }
    else if (type == Endpoint::RHS)
    {
        endpts.rhs.push_back (Endpoint (this, Endpoint::RHS, capacity, KEY(Pathway)));
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

    if (conattr.latency > 10000)
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

    // TODO to be implemented

    return icount;
}

void Pathway::PreClock (PERMIT(Simulator))
{
    if (ready)
    {
        operation ("sample from connection (LHS pop)")
        endpts.lhs[0].Pop ();
    }

    operation ("push messages (RHS push)")
    {    
        Message *sampledmsg = conn.Sample ();

        if (sampledmsg)
        {
            bool shouldstop = false;
            for (Endpoint &ept : endpts.rhs)
            {
                if (!ept->Assign (sampledmsg))
                {
                    SIM_WARNING ("message dropped (portname: %s)",
                            GetName().c_str(), ept->GetConnectedPortName().c_str());
                }

                if ((uint32_t)(ept->GetConnectedModule().IsStalled ())
                        + ept->GetCapacity () - ept->GetNumMessages ()
                        >= conn.conattr.latency + 1)
                    shouldstop = true;
            }

            next_ready = !shouldstop;
        }
    }
}

void Pathway::PostClock (PERMIT(Simulator))
{
    Message *msgtoflow = nullptr;
    if (ready)
    {
        operation ("sample message from LHS")
            msgtoflow = endpts.lhs[0].Peek ();
    }

    operation ("flow messages through connection")
    conn.Flow (msgtoflow);

    operation ("transition ready state")
    ready = next_ready;
}
