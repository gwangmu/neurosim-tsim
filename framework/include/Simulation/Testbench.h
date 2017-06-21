#pragma once

/* to be used in module declarations */
#define TESTBENCH_VISIBLE friend class Testbench

/* to be used at the bottom of testbench header */
#define EXPORT_TESTBENCH(TB) Testbench *simtb = new TB()

/* to be used in constructor */
#define TESTBENCH_NAME(name) clsname = name
#define SCRIPT_PATH(scr,path) fscrpaths[scr] = path
#define REGDATA_PATH(reg,path) regpaths[reg] = path
#define CLOCK_PERIOD(clk,period) clkperiods[clk] = period


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
    enum ParamType { FILESCRIPT_PATH, REGISTER_PATH, CLOCK_PERIOD };

public:
    Testbench (const char *clsname, string name) : Metadata (clsname, name) {};

    /* Called by 'Simulator' */
    virtual Component* LoadTopComponent (PERMIT(Simulator)) = 0;
    string GetStringParam (ParamType ptype, string pname, PERMIT(Simulator));
    uint32_t GetUIntParam (ParamType ptype, string pname, PERMIT(Simulator));

    virtual bool IsFinished (PERMIT(Simulator)) = 0;

protected:
    map<string, string> fscrpaths;
    map<string, string> regpaths;
    map<string, uint32_t> clkperiods;
};
    
