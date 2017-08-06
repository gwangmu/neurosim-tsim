#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Pathway/Endpoint.h>
#include <TSim/Interface/IValidatable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

namespace TSim
{
    class Simulator;
    class Testbench;
    class Pathway;
    class Unit;

    class Component: public Metadata, public IValidatable
    {
    public:
        static const uint32_t MAX_MODULE_PORTS = 64;
    
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
    
        string GetFullName ();
        string GetFullNameWOClass ();
        Component* GetParent () { return parent; }
        virtual uint32_t GetNumChildModules ();
        uint32_t GetNumDirectChildModules ();
    
        virtual string GetClock ();
        virtual set<string> GetClockSet ();
        uint32_t GetDisPower ();
        uint32_t GetStaticPower ();
        uint32_t GetDynamicPower ();
        double GetDirectDisPower ();
        double GetDirectStaticPower ();
        double GetDirectDynamicPower ();
    
        virtual Component* GetComponent (string name);
        virtual Unit* GetUnit (string name);
    
        void SetDissipationPower (uint32_t pow, PERMIT(Simulator)) { dispower = pow; }
        void SetStaticPower (uint32_t pow, PERMIT(Simulator)) { stapower = pow; }
        void SetDynamicPower (uint32_t pow, PERMIT(Simulator)) { dynpower = pow; }
        virtual CycleClass<double> GetAggregateCycleClass ();
        virtual EventCount<double> GetAggregateEventCount ();
        virtual double GetAggregateConsumedEnergy ();
    
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
        uint32_t GetNumDescendantModules ();
    
        vector<Pathway *>::iterator PathwayBegin () { return pathways.begin (); }
        vector<Pathway *>::iterator PathwayEnd () { return pathways.end (); }
        uint32_t GetNumDescendantPathways ();
    
        virtual string GetGraphVizBody (uint32_t level);
    
        virtual IssueCount Validate (PERMIT(Simulator));
    
        /* Called by parent 'Component' */
        virtual bool Connect (string portname, Endpoint *endpt);
    
        /* Called by child 'Component' */
        double CalcDirectDisPowerShare (Component *child);
        double CalcDirectStaticPowerShare (Component *child);
        double CalcDirectDynamicPowerShare (Component *child);
    
        /* Called by 'Pathway' */
        bool AddChildPathway (Pathway *pathway, PERMIT(Pathway));
        double GetDisPowerPerPathway ();
        double GetStaticPowerPerModule ();
        double GetDynamicPowerPerModule ();
    
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
        uint32_t dispower;
        uint32_t stapower;
        uint32_t dynpower;
    };
}
