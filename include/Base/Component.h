#pragma once

#include <Base/Component.h>
#include <Utility/AccessKey.h>

#include <string>
#include <vector>
#include <map>

using namespace std;

class Simulator;
class Pathway;
class Endpoint;


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
    Component* GetParent () { return parent; }

    /* Called by 'Simulator' */
    auto ChildBegin (PERMIT(Simulator)) { return children.begin (); }
    auto ChildEnd (PERMIT(Simulator)) { return children.end (); }

    auto PathwayBegin (PERMIT(Simulator)) { return pathways.begin (); }
    auto PathwayEnd (PERMIT(Simulator)) { return pathways.end (); }

    virtual IssueCount Validate (PERMIT(Simulator)) final;

protected:
    /* Called by parent 'Component' */
    bool SetClock (string clockname) { this->clockname = clockname; }
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
