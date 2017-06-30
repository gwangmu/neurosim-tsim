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
protected:
    static const uint32_t MAX_MODULE_PORTS = 64;
    
public:
    template <typename T>
    struct CycleClass
    {
        T active = 0;
        T idle = 0;
    };

    template <typename T>
    struct EventCount
    {
        T stalled = 0;
        T oport_full[MAX_MODULE_PORTS] = {0};
    };

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
    virtual uint32_t GetNumChildModules ();

    virtual Module* GetModule (string name);
    virtual CycleClass<double> GetAggregateCycleClass ();
    virtual EventCount<double> GetAggregateEventCount ();

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
    vector<Component *>::iterator ChildBegin () { return children.begin (); }
    vector<Component *>::iterator ChildEnd () { return children.end (); }

    vector<Pathway *>::iterator PathwayBegin () { return pathways.begin (); }
    vector<Pathway *>::iterator PathwayEnd () { return pathways.end (); }

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
