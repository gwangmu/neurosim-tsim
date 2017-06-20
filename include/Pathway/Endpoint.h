#pragma once

#include <Base/Metadata.h>
#include <Utility/AccessKey.h>

#include <string>
#include <cinttypes>
#include <queue>

using namespace std;

class Pathway;
class Module;
class Message;
class Simulator;


class Endpoint final: public Metadata
{
public:
    enum Type { LHS, RHS, /* Uni */ };

public:
    Endpoint (string name, Pathway *parent, Type type, 
            uint32_t capacity, PERMIT(Pathway));

    /* Universal */
    Type GetEndpointType () { return type; }
    Pathway* GetParent () { return parent; }
    Module* GetConnectedModule () { return modConn; }
    string GetConnectedPortName () { return portConn; }

    /* Called by 'Pathway' and 'Module' */
    // FIXME enforcing LHS, RHS caller classes?
    bool Assign (Message *msg);
    Message* Peek () { return msgque.front (); }
    void Pop () { msgque.pop (); }

    bool IsFull (); 
    bool IsEmpty () { return msgque.empty (); }
    uint32_t GetCapacity { return capacity; }
    uint32_t GetNumMessages { return msgque.size (); }

    /* Called by 'Module' */
    bool JoinTo (Module *module, string portname, PERMIT(Module));

private:
    Type type;
    Pathway* parent;

    Module *modConn;
    string portConn;

    uint32_t capacity;
    queue<Message *> msgque;
};
