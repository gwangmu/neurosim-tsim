#pragma once

/* to be used in module declarations */
#define VISIBLE_TO(TB) friend class TB

/* to be used at the bottom of testbench header */
#define EXPORT_TESTBENCH(TB) Testbench *simtb = new TB()

/* to be used in constructor */
#define SET_FILESCRIPT_PATH(scr,path) fscrpaths[scr] = path
#define SET_REGISTER_DATAPATH(reg,path) regpaths[reg] = path
#define SET_CLOCK_PERIOD(clk,period) clkperiods[clk] = period


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
    enum ParamType { FILESCRIPT_PATH, REGISTER_DATAPATH, CLOCK_PERIOD };

public:
    Testbench (const char *clsname, Component *topcomp)
        : Metadata (clsname, ""), TOP_COMPONENT (topcomp) {}

    /* Called by 'Simulator' */
    virtual Component *const GetTopComponent (PERMIT(Simulator)) { return TOP_COMPONENT; }
    bool LoadSimulationSpec (string specfilename, PERMIT(Simulator));

    string GetStringParam (ParamType ptype, string pname, PERMIT(Simulator));
    uint32_t GetUIntParam (ParamType ptype, string pname, PERMIT(Simulator));

    virtual bool IsFinished (PERMIT(Simulator)) = 0;

protected:
    Component *const TOP_COMPONENT;

    map<string, string> fscrpaths;
    map<string, string> regpaths;
    map<string, uint32_t> clkperiods;
};
    
