#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Interface/IValidatable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <vector>
#include <map>

using namespace std;

class Simulator;
class Testbench;
class Pathway;


class Component: public Metadata, public IValidatable
{
private:
    struct PortExportTag
    {
        Component *child;
        string portname;
    };

public:
    /* Universal */
    Component (const char* clsname, string iname, Component *parent);

    string GetClock ();
    string GetFullName ();
    string GetFullNameWOClass ();
    Component* GetParent () { return parent; }

    virtual Module* GetModule (string name);

    bool IsAncestor (Component *comp) 
    { return (!parent) ? false : parent->IsAncestor (comp); }
    bool IsDescendant (Component *comp)
    { 
        if (!children.empty ())
        {
            for (Component *ccomp : children)
                if (ccomp->IsDescendant (comp))
                    return true;
        }
        return false;
    }

    /* Called by 'Simulator' */
    vector<Component *>::iterator ChildBegin (PERMIT(Simulator)) { return children.begin (); }
    vector<Component *>::iterator ChildEnd (PERMIT(Simulator)) { return children.end (); }

    vector<Pathway *>::iterator PathwayBegin (PERMIT(Simulator)) { return pathways.begin (); }
    vector<Pathway *>::iterator PathwayEnd (PERMIT(Simulator)) { return pathways.end (); }

    virtual IssueCount Validate (PERMIT(Simulator));

    /* Called by parent 'Component' */
    virtual bool Connect (string portname, Endpoint *endpt);

    /* Called by 'Pathway' */
    bool AddChildPathway (Pathway *pathway, PERMIT(Pathway));

protected:
    /* Called by parent 'Component' */
    bool SetClock (string clockname);

    /* Called by this 'Component' */
    virtual bool ExportPort (string eportname, Component *child, string cportname);

    /* Called by child 'Component' */
    virtual bool AddChild (Component *child);

private:
    Component *parent;

    map<string, PortExportTag> portname2tag;
    vector<Component *> children;
    vector<Pathway *> pathways;

    string clockname;
};
