#pragma once

/* to be used in module declarations */
#define VISIBLE_TO(TB) friend class TB

/* to be used at the bottom of testbench header */
#define EXPORT_TESTBENCH(TB) Testbench *simtb = new TB()

/* to be used by module constructors */
#define USING_TESTBENCH extern Testbench *simtb
#define IMPORT_PARAMETER(x, def) {                  \
    x = simtb->GetUIntParam (Testbench::PARAMETER,  \
            GetInstanceName() + "." + #x);          \
    if (x == -1) x = def;                           \
}

#include <TSim/Base/Metadata.h>

#include <TSim/Utility/AccessKey.h>
#include <cinttypes>
#include <string>
#include <map>

using namespace std;

class Simulator;
class Component;


class Testbench: public Metadata
{
public:
    enum ParamType 
    { 
        FILESCRIPT_PATH, 
        REGISTER_DATAPATH, 
        CLOCK_PERIOD,
        UNIT_DYNAMIC_POWER,
        UNIT_STATIC_POWER,
        PATHWAY_DIS_POWER,
        PARAMETER
    };

public:
    Testbench (const char *clsname, Component *topcomp)
        : Metadata (clsname, ""), TOP_COMPONENT (topcomp) {}

    /* Called by 'Simulator' */
    virtual Component *const GetTopComponent (PERMIT(Simulator)) { return TOP_COMPONENT; }
    bool LoadSimulationSpec (string specfilename, PERMIT(Simulator));

    string GetStringParam (ParamType ptype, string pname);
    uint32_t GetUIntParam (ParamType ptype, string pname);

    virtual bool IsFinished (PERMIT(Simulator)) = 0;

protected:
    Component *const TOP_COMPONENT;

    map<string, string> fscrpaths;
    map<string, string> regpaths;
    map<string, uint32_t> clkperiods;
    map<string, uint32_t> moddynpow;
    map<string, uint32_t> modstapow;
    map<string, uint32_t> pathdispow;
    map<string, uint32_t> modparams;
};
    
