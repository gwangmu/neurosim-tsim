#include <TSim/Base/Component.h>

#include <TSim/Simulation/Simulator.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <string>
#include <vector>
#include <map>

using namespace std;


Component::Component (const char *clsname, string name, Component *parent)
    : Metadata (clsname, name)
{
    this->parent = parent;
    if (parent)
        parent->AddChild (this);

    this->clockname = "";
}


string Component::GetClock ()
{ 
    if (!clockname.empty ())
        return clockname;
    else if (!parent)
        return "";
    else
        return parent->GetClock();
}

string Component::GetFullName ()
{
    return (string(GetClassName()) + " " + GetFullNameWOClass());
}

string Component::GetFullNameWOClass ()
{
    string familyname = "";
    if (parent)
        familyname = parent->GetFullNameWOClass ();

    if (familyname.empty ())
        return GetInstanceName();
    else
        return (familyname + "::" + GetInstanceName());
}


Module* Component::GetModule (string name)
{
    Module *tar = nullptr;
    for (Component *child : children)
    {
        if ((tar = child->GetModule (name)))
            break;
    }

    return tar;
}


bool Component::AddChildPathway (Pathway *pathway, PERMIT(Pathway))
{
    for (Pathway *cpathway : pathways)
    {
        if (cpathway == pathway)
        {
            DESIGN_WARNING ("already has '%s' as a child. ignoring",
                    GetFullName().c_str(), pathway->GetName().c_str());
            return true;
        }
    }

    pathways.push_back (pathway);

    return true;
}


IssueCount Component::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (GetClock().empty ())
    {
        DESIGN_ERROR ("no clock assigned", GetFullName().c_str());
        icount.error++;
    }

    for (Component *comp : children)
    {
        IssueCount subicount = comp->Validate (TRANSFER_KEY(Simulator));
        icount.error += subicount.error;
        icount.warning += subicount.error;
    }

    for (Pathway *pathway : pathways)
    {
        IssueCount subicount = pathway->Validate (TRANSFER_KEY(Simulator));
        icount.error += subicount.error;
        icount.warning += subicount.error;
    }

    return icount;
}


bool Component::SetClock (string clockname)
{
    if (!this->clockname.empty ())
        DESIGN_WARNING ("changing clock '%s' to '%s'",
                GetFullName().c_str(), this->clockname.c_str(), clockname.c_str());

    this->clockname = clockname;
    return true;
}

bool Component::Connect (string portname, Endpoint *endpt)
{
    if (portname2tag.count (portname))
    {
        DESIGN_ERROR ("non-existing port '%s'",
                GetFullName().c_str(), portname.c_str());
        return false;
    }

    if (!portname2tag[portname].child) {
        SYSTEM_ERROR ("null exported port (who: %s, portname: %s)",
                GetFullName().c_str(), portname2tag[portname].portname.c_str());
        return false;
    }

    return portname2tag[portname].child->Connect (
            portname2tag[portname].portname, 
            endpt);
}

bool Component::ExportPort (string eportname, Component *child, string cportname)
{
    if (child == nullptr)
    {
        DESIGN_ERROR ("cannot export null child's port",
                GetFullName().c_str());
        return false;
    }

    if (portname2tag.count (eportname))
    {
        DESIGN_WARNING ("port '%s' already exist. overwriting",
                GetFullName().c_str(), eportname.c_str());
    }

    PortExportTag tag;
    tag.child = child;
    tag.portname = cportname;

    portname2tag[eportname] = tag;

    return true;
}

bool Component::AddChild (Component *child)
{
    if (child == nullptr)
    {
        DESIGN_ERROR ("cannot add null child", GetFullName().c_str());
        return false;
    }

    if (IsAncestor (child) || IsDescendant (child))
    {
        DESIGN_ERROR ("%s is already in the hierarchy",
                GetFullName().c_str(), child->GetFullName().c_str());
        return false;
    }

    children.push_back (child);

    return true;
}
