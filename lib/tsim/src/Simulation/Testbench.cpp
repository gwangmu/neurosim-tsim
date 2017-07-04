#include <TSim/Simulation/Testbench.h>

#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/String.h>

#include <map>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;


bool Testbench::LoadSimulationSpec (string specfilename, PERMIT(Simulator))
{
    ifstream specfile;

    task ("open simulation spec file") 
    {
        specfile.open (specfilename);
        if (!specfile.is_open ()){
            DESIGN_ERROR ("cannot open simulation spec file '%s'",
                    GetName().c_str(), specfilename.c_str());
            return false;
        }
    }

    task ("parse spec file") 
    {
        uint32_t lineno = 1;
        string line;

        while (getline (specfile, line))
        {
            line = String::Trim (line);

            if (line[0] == '#') continue;
            if (line == "") continue;

            vector<string> toked = String::Tokenize (line, ":()");
            if (toked[0] == "FILESCRIPT_PATH")
                SET_FILESCRIPT_PATH (toked[1], toked[2]);
            else if (toked[0] == "REGISTER_DATAPATH")
                SET_REGISTER_DATAPATH (toked[1], toked[2]);
            else if (toked[0] == "CLOCK_PERIOD")
                SET_CLOCK_PERIOD (toked[1], stoi (toked[2]));
            else if (toked[0] == "UNIT_DYNAMIC_POWER")    
                SET_UNIT_DYNAMIC_POWER (toked[1], stoi (toked[2]));
            else if (toked[0] == "UNIT_STATIC_POWER")    
                SET_UNIT_STATIC_POWER (toked[1], stoi (toked[2]));
            else if (toked[0] == "PATHWAY_DIS_POWER")    
                SET_PATHWAY_DIS_POWER (toked[1], stoi (toked[2]));
            else
            {
                SIM_ERROR ("unknown parameter name '%s' (regdata: %s, lineno: %u)",
                        GetName().c_str(), toked[0].c_str(), specfilename.c_str(), lineno);
                return false;
            }
                DEBUG_PRINT ("parameter (%s <-- %s)", toked[1].c_str(), toked[2].c_str());

            lineno++;
        }

        specfile.close ();
    }

    return true;
}


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
    else if (ptype == Testbench::UNIT_DYNAMIC_POWER)
    {
        if (moddynpow.count (pname))
            return moddynpow[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::UNIT_STATIC_POWER)
    {
        if (modstapow.count (pname))
            return modstapow[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::PATHWAY_DIS_POWER)
    {
        if (pathdispow.count (pname))
            return pathdispow[pname];
        else
            return -1;
    }
    else
        SYSTEM_ERROR ("parameter '%u' is not an integer", ptype);
}
