#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Utility/AccessKey.h>

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
    enum Type { LHS, RHS };

public:
    Endpoint (string name, Pathway *parent, Type type, 
            uint32_t capacity, PERMIT(Pathway));

    /* Universal */
    Type GetEndpointType () { return type; }
    Pathway* GetParent () { return parent; }
    Module* GetConnectedModule () { return modConn; }
    string GetConnectedPortName () { return portConn; }

    /* Called by 'Component' */
    void SetCapacity (uint32_t capacity);

    /* Called by 'Pathway' and 'Module' (during simulation) */
    // FIXME enforcing LHS, RHS caller classes?
    bool Assign (Message *msg);
    Message* Peek () { return msgque.front (); }
    void Pop () { msgque.pop (); }

    bool IsFull (); 
    bool IsEmpty () { return msgque.empty (); }
    uint32_t GetCapacity () { return capacity; }
    uint32_t GetNumMessages () { return msgque.size (); }

    void SetSelectedLHS (bool val) { selected_lhs = val; }
    bool IsSelectedLHSOfThisCycle () { return selected_lhs; }

    /* Called by 'Module' */
    bool JoinTo (Module *module, string portname, PERMIT(Module));

private:
    Type type;
    Pathway* parent;

    Module *modConn;
    string portConn;

    uint32_t capacity;
    queue<Message *> msgque;
    bool selected_lhs;
};
