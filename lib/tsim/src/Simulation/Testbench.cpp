#include <TSim/Simulation/Testbench.h>

#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <map>
#include <string>

using namespace std;


string Testbench::GetStringParam (Testbench::ParamType ptype, 
        string pname, PERMIT(Simulator))
{
    if (ptype == Testbench::FILESCRIPT_PATH)
    {
        if (fscrpaths.count (pname))
            return fscrpaths[pname];
        else
            return "";
    }
    else if (ptype == Testbench::REGISTER_DATAPATH)
    {
        if (regpaths.count (pname))
            return regpaths[pname];
        else
            return "";
    }
    else
        SYSTEM_ERROR ("parameter '%u' is not a string", ptype);
}

uint32_t Testbench::GetUIntParam (Testbench::ParamType ptype,
        string pname, PERMIT(Simulator))
{
    if (ptype == Testbench::CLOCK_PERIOD)
    {
        if (clkperiods.count (pname))
            return clkperiods[pname];
        else
            return -1;
    }
    else
        SYSTEM_ERROR ("parameter '%u' is not an integer", ptype);
}
