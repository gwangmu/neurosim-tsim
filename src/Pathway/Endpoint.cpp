#include <Pathway/Endpoint.h>

#include <Pathway/Pathway.h>
#include <Pathway/Message.h>
#include <Module/Module.h>
#include <Simulation/Simulator.h>
#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

#include <string>
#include <cinttypes>
#include <queue>

using namespace std;


Endpoint::Endpoint (Pathway *parent, Type type, uint32_t capacity, PERMIT(Pathway))
    : Metadata ("Endpoint", "")
{
    this->type = type;
    this->parent (parent);

    this->modConn = nullptr;
    this->portConn = "";

    this->capacity = capacity;
}


bool Endpoint::Assign (Message *msg)
{
    if (msgque.size () == capacity)
    {
        SYSTEM_WARNING ("attemped to enque to full endpoint");
        return false;
    }
    
    msgque.push (msg);
    return true;
}

bool Endpoint::IsFull () 
{ 
    #ifndef NDEBUG
    if (msgque.size > capacity)
        SYSTEM_ERROR ("endpoint queue size exceeded capacity");
    #endif

    return msgque.size() >= capacity;
}


bool Endpoint::JoinTo (Module *module, string portname, PERMIT(Module))
{
    if (modConn != nullptr)
    {
        DESIGN_WARNING ("port '%s' (of %s) has been already asigned",
                GetClassName(), portname.c_str(), module->GetFullName ());
    }

    modConn = module;
    portConn = portname;
    return true;
}
