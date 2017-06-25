#include <Module/Module.h>

#include <Base/Component.h>
#include <Script/Script.h>
#include <Register/Register.h>
#include <Pathway/Endpoint.h>
#include <Pathway/Pathway.h>
#include <Pathway/Message.h>
#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

#include <cinttypes>
#include <vector>
#include <string>
#include <map>

using namespace std;


/* Constructors */
Module::Port::Port ()
{
    name = "";
    iotype = Module::Port::UNKNOWN;
    msgproto = nullptr;
    endpt = nullptr;
}

Module::Module (const char *clsname, string iname, Component *parent):
    Component (clsname, iname, parent)
{
    script = nullptr;
    reg = nullptr;
    outMsgPended = false;
}


/* functions for 'Component' */
bool Module::SetScript (Script *script)
{
    if (this->script)
        DESIGN_WARNING ("overwriting '%s' with '%s'",
                GetFullName().c_str(), this->script->GetName().c_str(), 
                script->GetName().c_str());

    if (script->IsAssigned ()) {
        DESIGN_ERROR ("'%s' has already been assigned",
                GetFullName().c_str(), script->GetName().c_str());
        return false;
    }

    this->script = script;

    return true;
}

bool Module::SetRegister (Register *reg)
{
    if (this->reg)
        DESIGN_WARNING ("overwriting '%s' with '%s'",
                GetFullName().c_str(), this->reg->GetName().c_str(),
                reg->GetName().c_str());

    if (reg->IsAssigned ()) {
        DESIGN_ERROR ("'%s' has already been assigned",
                GetFullName().c_str(), reg->GetName().c_str());
        return false;
    }

    this->reg = reg;

    return true;
}


bool Module::Connect (string portname, Endpoint *endpt)
{
    if (!pname2port.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'", GetFullName().c_str(), 
                portname.c_str());
        return false;
    }

    Port *port = pname2port[portname];

    if (port->iotype == Module::Port::UNKNOWN)
    {
        SYSTEM_ERROR ("port type cannot be UNKNOWN"
                "(portname: %s)", portname.c_str());
        return false;
    }

    if (!endpt)
    {
        DESIGN_WARNING ("attemping to assign a null endpoint to the port '%s'",
                GetFullName().c_str(), portname.c_str());
        port->endpt = nullptr;
        return true;
    }

    if (endpt->GetParent()->GetMsgPrototype() != port->msgproto)
    {
        DESIGN_ERROR ("mismatching message proto %s (of '%s') "
                "and %s (of port '%s')",
                GetFullName().c_str(), 
                endpt->GetParent()->GetMsgPrototype()->GetClassName (),
                endpt->GetParent()->GetName().c_str(), 
                port->msgproto->GetClassName (), portname.c_str());
        return false;
    }

    if (!(port->iotype == Module::Port::INPUT &&
                endpt->GetEndpointType() == Endpoint::RHS) &&
            !(port->iotype == Module::Port::OUTPUT &&
                endpt->GetEndpointType() == Endpoint::LHS))
    {
        DESIGN_ERROR ("incompatible endpoint '%s' to %s port '%s'",
                GetName().c_str(), endpt->GetClassName(),
                Module::Port::GetTypeString(port->iotype), portname.c_str());
        return false;
    }

    port->endpt = endpt;
    endpt->JoinTo (this, portname, KEY(Module));

    return true;
}


Module* Module::GetModule (string name)
{
    if (GetInstanceName() == name)
        return this;
    else
        return nullptr;
}


/* functions for 'Simulator' */
IssueCount Module::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!GetParent())
    {
        SYSTEM_ERROR ("module '%s' has no parent", GetFullName().c_str());
        icount.error++;
    }

    for (Port &port : inports)
    {
        if (!port.endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
            icount.warning++;
        }
    }
    
    for (Port &port : outports)
    {
        if (!port.endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", 
                    GetFullName().c_str(), port.name.c_str());
            icount.warning++;
        }
    }

    return icount;
}

/*>>> ! PERFORMANCE-CRITICAL ! <<<*/
void Module::PreClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("pre-clocking '%s'", GetFullName().c_str());

    if (outMsgPended)
    {
        Instruction *nextinstr = nullptr;
        operation ("sample messages (RHS peek)")
        {
            for (auto i = 0; i < inports.size(); i++)
                if (!nextinmsgs[i])
                    nextinmsgs[i] = inports[i].endpt->Peek ();
            
            if (script)
                script->NextInstruction ();
        }

        operation ("call operation")
        {
            // NOTE: set nextinmsgs[i] to nullptr to pop ith input
            // NOTE: assign new message to nextoutmsgs[j] to push to jth output
            Operation (nextinmsgs, nextoutmsgs, nextinstr);
        }
        
        operation ("pop messages (RHS pop)");
        for (auto i = 0; i < inports.size(); i++)
            if (!nextinmsgs[i])
                inports[i].endpt->Pop ();
    }

    return;
}

/*>>> ! PERFORMANCE-CRITICAL ! <<<*/
void Module::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("post-clocking '%s'", GetFullName().c_str());

    operation ("dispose incoming messages")
    {
        for (auto i = 0; i < inports.size (); i++)
            nextinmsgs[i]->Dispose (KEY(Module));
    }

    operation ("check pending state")
    {
        for (Port &port : outports)
        {
            if (!port.endpt->IsFull ())
            {
                outMsgPended = true;
                return;
            }
        }
    }

    operation ("assign/clear messages (LHS push)")
    {
        for (auto i = 0; i < outports.size (); i++)
        {
            if (nextoutmsgs[i])
            {
                outports[i].endpt->Assign (nextoutmsgs[i]);
                nextoutmsgs[i] = nullptr;
            }
        }
    }

    outMsgPended = false;

    return;
}


/* Called by 'Module' */
uint32_t Module::CreatePort (string portname, Module::Port::Type iotype,
        Message* msgproto)
{
    uint32_t id = -1;
    Port *port = nullptr;

    if (iotype == Module::Port::INPUT)
    {
        inports.push_back (Port ());
        id = inports.size () - 1;
        port = &inports.back ();
        
        delete[] nextinmsgs;
        nextinmsgs = new Message *[inports.size ()] ();
    }
    else if (iotype == Module::Port::OUTPUT)
    {
        outports.push_back (Port ());
        id = outports.size () - 1;
        port = &outports.back ();
        
        delete[] nextoutmsgs;
        nextoutmsgs = new Message *[outports.size ()] ();
    }
    else
    {
        DESIGN_ERROR ("cannot create %s port", GetFullName().c_str(),
                Module::Port::GetTypeString (iotype));
        return -1;
    }

    port->name = portname;
    port->id = id;
    port->iotype = iotype;
    port->msgproto = msgproto;
    
    port->endpt = nullptr;

    return id;
}
