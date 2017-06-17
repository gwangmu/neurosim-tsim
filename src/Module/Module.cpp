#include <Module/Module.h>

#include <Component/Component.h>
#include <Script/Script.h>
#include <Register/Register.h>
#include <Pathway/Endpoint.h>
#include <Pathway/Pathway.h>
#include <Pathway/Message.h>
#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

#include <inttypes.h>
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

Module::Module (Component *parent, const char *clsname, string name)
{
    this->parent = parent;
    if (parent)
        parent->AddChild (this, KEY(Interface));

    this->name = name;
    this->clsname = clsname;

    script = nullptr;
    reg = nullptr;
    outMsgPended = false;
}


/* functions from 'IMetadata' */
string Module::GetFullName ()
{
    string familyname = "";
    if (parent)
        familyname = parent->GetFullName ();

    return (familyname + string("::") + name);
}

string Module::GetSummary ()
{
    // TODO to be implemented
}


/* functions for 'Component' */
bool Module::SetScript (Script *script)
{
    if (this->script)
        DESIGN_WARNING ("overwriting '%s %s' with '%s %s'",
                name.c_str(), this->script->GetClassName(), 
                this->script->GetName().c_str(),
                script->GetClassName(), script->GetName().c_str());

    if (script->IsAssigned ()) {
        DESIGN_ERROR ("'%s %s' has already been assigned",
                name.c_str(), script->GetClassName(), script->GetName().c_str());
        return false;
    }

    this->script = script;

    return true;
}

bool Module::SetRegister (Register *reg)
{
    if (this->reg)
        DESIGN_WARNING ("overwriting '%s %s' with '%s %s'",
                name.c_str(), this->reg->GetClassName(), this->reg->GetName().c_str(),
                reg->GetClassName(), reg->GetName().c_str());

    if (reg->IsAssigned ()) {
        DESIGN_ERROR ("'%s %s' has already been assigned",
                name.c_str(), reg->GetClassName(), reg->GetName().c_str());
        return false;
    }

    this->reg = reg;

    return true;
}


bool Module::Connect (string portname, Endpoint *endpt)
{
    if (!portname2id.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'", name.c_str(), portname.c_str());
        return false;
    }

    Port *port = pname2port[portname];

    if (port->iotype == Module::Port::UNKNOWN)
    {
        SYSTEM_ERROR ("port type cannot be UNKNOWN"
                "(portname: %s, portid: %u)", portname.c_str(), portid);
        return false;
    }

    if (!endpt)
    {
        DESIGN_WARNING ("attemping to assign a null endpoint", name);
        port->endpt = nullptr;
        return true;
    }

    if (typeid(endpt->GetParent()->GetMsgPrototype ())
            != typeid(port->msgproto))
    {
        DESIGN_ERROR ("mismatching message proto %s (of %s %s) "
                "and %s (of port '%s')",
                name.c_str(), endpt->GetParent()->GetMsgPrototype()->GetClassName (),
                endpt->GetParent()->GetClassName (), 
                endpt->GetParent()->GetFullName().c_str (),
                port->msgproto->GetClassName (), portname.c_str());
        return false;
    }

    if (!(port->iotype == Module::Port::INPUT &&
                dynamic_cast<RHSEndpoint *>(endpt)) &&
            !(port->iotype == Module::Port::Output &&
                dynamic_cast<LHSEndpoint *>(endpt)))
    {
        DESIGN_ERROR ("incompatible endpoint '%s %s' to %s port '%s'",
                name.c_str(), endpt->GetClassName(), endpt->GetFullName().c_str (),
                Module::Port::GetIOTypeString(port->iotype), portname.c_str());
        return false;
    }

    port->endpt = endpt;

    return true;
}


/*>>> ! PERFORMANCE-CRITICAL ! <<<*/
/* functions for 'Simulator' */
IssueCount Module::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!parent)
    {
        SYSTEM_ERROR ("module '%s %s' has no parent", GetClassName(), name.c_str());
        icount.error++;
    }

    for (Port &port : ports)
    {
        if (!port.endpt)
        {
            DESIGN_WARNING ("disconnected port '%s'", name, port.name.c_str());
            icount.warning++;
        }
    }

    return icount;
}

void Module::PreClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("pre-clocking '%s %s'", GetClassName(), GetFullName().c_str());

    if (outMsgPended)
    {
        Instruction *nextinstr = nullptr;
        operation ("sample messages")
        {
            for (auto i = 0; i < inports.size(); i++)
                nextinmsgs[i] = inports[i].endpt->Fetch();
            
            if (script)
                script->NextInstruction ();
        }

        operation ("call operation");
        Operation (nextinmsgs, nextoutmsgs, nextinstr);
    }

    return;
}

void Module::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("post-clocking '%s %s'", GetClassName(), GetFullName().c_str());

    operation ("check pending state")
    {
        for (Port &port : outports)
        {
            if (!port->IsAssignable ())
            {
                outMsgPended = true;
                return;
            }
        }
    }

    operation ("assign/clear messages")
    {
        for (auto i = 0; i < outports.size (); i++)
        {
            if (nextoutmsgs[i])
            {
                outports[i]->Assign (nextoutmsgs[i]);
                nextoutmsgs[i] = nullptr;
            }
        }
    }

    outMsgPended = false;

    return;
}


/* Called by 'Module' */
uint32_t Module::CreatePort (string portname, Module::Port::IOType iotype,
        Message* msgproto)
{
    uint32_t id = -1;
    Port *port = nullptr;

    if (iotype == Module::Port::INPUT)
    {
        inports.push_back (Port ());
        id = inports.size () - 1;
        port = &inports.back ();
    }
    else if (iotype == Module::Port::OUTPUT)
    {
        outports.push_back (Port ());
        id = outports.size () - 1;
        port = &outports.back ();
    }
    else
    {
        DESIGN_ERROR ("cannot create port type '%s'", name.c_str(),
                Module::Port::GetIOTypeString (iotype));
        return -1;
    }

    port->name = portname;
    port->id = id;
    port->iotype = iotype;
    port->msgproto = msgproto;
    
    port->endpt = nullptr;

    return id;
}
