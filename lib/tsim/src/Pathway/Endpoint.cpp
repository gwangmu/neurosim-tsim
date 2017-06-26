#include <TSim/Pathway/Endpoint.h>

#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Module/Module.h>
#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

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

    this->selected_lhs = false;
}


bool Endpoint::SetCapacity (uint32_t capacity)
{
    if (capacity == 0)
    {
        DESIGN_ERROR ("endpoint must have capacity >= 1", GetName().c_str());
        return false;
    }

    this->capacity = capacity;
    return true;
}


bool Endpoint::Assign (Message *msg)
{
    if (capacity != 0 && msgque.size () == capacity)
    {
        SIM_WARNING ("attemped to enque to full endpoint", GetName().c_str());
        return false;
    }
    else if (capacity == 0 && msgque.size () >= 1)
    {
        SYSTEM_ERROR ("capacity=0 endpoint cannot have elements more than one");
        return false;
    }
    
    msgque.push (msg);
    return true;
}

bool Endpoint::IsFull () 
{ 
    #ifndef NDEBUG
    if (msgque.size () > capacity)
        SYSTEM_ERROR ("endpoint queue size exceeded capacity");
    #endif

    return msgque.size() >= capacity;
}


bool Endpoint::JoinTo (Module *module, string portname, PERMIT(Module))
{
    if (modConn != nullptr)
    {
        DESIGN_WARNING ("port '%s' (of %s) has been already asigned",
                GetName().c_str(), portname.c_str(), module->GetName().c_str());
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
