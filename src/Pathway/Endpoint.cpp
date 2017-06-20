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


Endpoint::Endpoint (string name, Pathway *parent, Type type, 
        uint32_t capacity, PERMIT(Pathway))
    : Metadata ("Endpoint", name)
{
    this->type = type;

    if (!parent)
        SYSTEM_ERROR ("endpoint '%s' with null parent", name.c_str());
    this->parent = parent;

    this->modConn = nullptr;
    this->portConn = "";

    if (capacity == 0)
        DESIGN_FATAL ("zero-capacity endpoint not allowed", GetName().c_str());
    this->capacity = capacity;
}


bool Endpoint::Assign (Message *msg)
{
    if (msgque.size () == capacity)
    {
        SIM_WARNING ("attemped to enque to full endpoint");
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
                GetName().c_str(), portname.c_str(), module->GetFullName ());
    }

    if (capacity == 0)
    {
        DESIGN_ERROR ("port '%s' has zero-capacity. cannot be jointed",
                GetName().c_str(), portname.c_str());
        return false;
    }

    modConn = module;
    portConn = portname;
    return true;
}
