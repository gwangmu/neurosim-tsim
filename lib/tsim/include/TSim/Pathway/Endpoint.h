#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <queue>

using namespace std;

class Module;
class Message;
class Simulator;


class Endpoint final: public Metadata
{
public:
    enum Type { CAP, LHS, RHS };

public:
    Endpoint (string name, Pathway *parent, Type type, 
            uint32_t capacity, PERMIT(Pathway));

    static inline Endpoint *const PORTCAP ()
    {
        if (unlikely (!_PORTCAP)) _PORTCAP = new Endpoint ();
        return _PORTCAP;
    }

    /* Universal */
    Type GetEndpointType () { return type; }
    Pathway* GetParent () { return parent; }
    Module* GetConnectedModule () { return modConn; }
    string GetConnectedPortName () { return portConn; }
    bool IsPortCap () { return (type == CAP); }

    /* Called by 'Component' */
    void SetCapacity (uint32_t capacity);

    /* Called by 'Pathway' and 'Module' (during simulation) */
    // FIXME enforcing LHS, RHS caller classes?
    void Reserve ();
    bool Assign (Message *msg);
    Message* Peek () { return (msgque.empty() ? nullptr : msgque.front ()); }
    void Pop () 
    { 
        if (unlikely (msgque.empty()))
           SYSTEM_ERROR ("cannot pop from empty queue (endpt: %s)", GetName().c_str());
        msgque.pop (); 
    }

    bool IsFull (); 
    bool IsOverloaded ();
    bool IsEmpty () { return msgque.empty (); }
    uint32_t GetCapacity () { return capacity; }
    uint32_t GetNumReserved () { return resv_count; }
    uint32_t GetNumMessages () { return msgque.size (); }

    void SetSelectedLHS (bool val) { selected_lhs = val; }
    bool IsSelectedLHSOfThisCycle () { return selected_lhs; }

    /* Called by 'Module' */
    bool JoinTo (Module *module, string portname, PERMIT(Module));

private:
    /* For PORTCAP */
    Endpoint ();
    static Endpoint *_PORTCAP;

    Type type;
    Pathway* parent;

    Module *modConn;
    string portConn;

    uint32_t capacity;
    queue<Message *> msgque;
    uint32_t resv_count;

    bool selected_lhs;
};
