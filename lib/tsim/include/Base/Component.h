#pragma once

#include <Base/Metadata.h>
#include <Pathway/Endpoint.h>
#include <Interface/IValidatable.h>
#include <Utility/AccessKey.h>

#include <string>
#include <vector>
#include <map>

using namespace std;

class Simulator;
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
    auto ChildBegin (PERMIT(Simulator)) { return children.begin (); }
    auto ChildEnd (PERMIT(Simulator)) { return children.end (); }

    auto PathwayBegin (PERMIT(Simulator)) { return pathways.begin (); }
    auto PathwayEnd (PERMIT(Simulator)) { return pathways.end (); }

    virtual IssueCount Validate (PERMIT(Simulator));

    /* Called by 'Pathway' */
    bool AddChildPathway (Pathway *pathway, PERMIT(Pathway));

protected:
    /* Called by parent 'Component' */
    bool SetClock (string clockname);
    virtual bool Connect (string portname, Endpoint *endpt);

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
