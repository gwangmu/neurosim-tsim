#pragma once

/* to be used in module declarations */
#define TESTBENCH_VISIBLE friend class Testbench

/* to be used at the bottom of testbench header */
#define EXPORT_TESTBENCH(TB) Testbench *simtb = new TB()

/* to be used in constructor */
#define SET_FILESCRIPT_PATH(scr,path) fscrpaths[scr] = path
#define SET_REGISTER_DATAPATH(reg,path) regpaths[reg] = path
#define SET_CLOCK_PERIOD(clk,period) clkperiods[clk] = period


#include <Base/Metadata.h>

#include <Utility/AccessKey.h>
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
    virtual Component *const LoadTopComponent (PERMIT(Simulator)) { return TOP_COMPONENT; }
    string GetStringParam (ParamType ptype, string pname, PERMIT(Simulator));
    uint32_t GetUIntParam (ParamType ptype, string pname, PERMIT(Simulator));

    virtual bool IsFinished (PERMIT(Simulator)) = 0;

protected:
    Component *const TOP_COMPONENT;

    map<string, string> fscrpaths;
    map<string, string> regpaths;
    map<string, uint32_t> clkperiods;
};
    
