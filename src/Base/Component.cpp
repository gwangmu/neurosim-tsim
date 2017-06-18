#include <Base/Component.h>

#include <Simulation/Simulator.h>
#include <Pathway/Pathway.h>
#include <Pathway/Endpoint.h>
#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

#include <string>
#include <vector>
#include <map>

using namespace std;


Component::Component (const char *clsname, string name, Component *parent)
{
    if (!clsname)
        DESIGN_FATAL ("class name cannot be null", "LogicBlock");

    this->clsname = clsname; 

    if (name.empty ())
        this->name = "(noname)";
    else
        this->name = name;

    this->parent = parent;
    if (parent)
        parent->AddChild (this);

    this->clockname = "";
}


string Component::GetClock ()
{ 
    if (!clock.empty ())
        return clockname;
    else if (!parent)
        return "";
    else
        return parent->GetClock();
}


IssueCount Component::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (GetClock().empty ())
    {
        DESIGN_ERROR ("no clock assigned", GetFullName().c_str());
        icount++;
    }

    for (LogicBlock *logicblk : children)
    {
        IssueCount subicount = logicblk->Validate (TRANSFER_KEY(Simulator));
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
