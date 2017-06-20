#include <Simulation/Testbench.h>

#include <Simulation/Simulator.h>
#include <Utility/AccessKey.h>
#include <Utility/Logging.h>

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
    else if (ptype == Testbench::REGISTER_PATH)
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
            return "";
    }
    else
        SYSTEM_ERROR ("parameter '%u' is not an integer", ptype);
}
