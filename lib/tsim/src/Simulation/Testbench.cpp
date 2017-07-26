#define SET_FILESCRIPT_PATH(scr,path) fscrpaths[scr] = path
#define SET_REGISTER_DATAPATH(reg,path) regpaths[reg] = path
#define SET_CLOCK_PERIOD(clk,period) clkperiods[clk] = period
#define SET_UNIT_DYNAMIC_POWER(mod,pow) moddynpow[mod] = pow
#define SET_UNIT_STATIC_POWER(mod,pow) modstapow[mod] = pow
#define SET_COMPONENT_DIS_POWER(comp,pow) compdispow[comp] = pow
#define SET_REGISTER_READ_ENERGY(mod,pow) regrdenergy[mod] = pow
#define SET_REGISTER_WRITE_ENERGY(mod,pow) regwrenergy[mod] = pow
#define SET_REGISTER_STATIC_POWER(reg,pow) regstapow[reg] = pow
#define SET_PARAMETER(pname,param) modparams[pname] = param

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

        // TODO: multi-variable support
        uint32_t range_start = -1;
        uint32_t range_end = -1;

        while (getline (specfile, line))
        {
            line = String::Trim (line);

            if (line[0] == '#') continue;
            if (line == "") continue;

            vector<string> toked = String::Tokenize (line, ":()");
            if (toked[0] == "FILESCRIPT_PATH")
            {
                if (range_start == -1 || range_end == -1)
                    SET_FILESCRIPT_PATH (toked[1], toked[2]);
                else
                {
                    for (uint32_t i = range_start; i <= range_end; i++)
                    {
                        string lhs = String::Replace (toked[1], "@", to_string(i));
                        string rhs = String::Replace (toked[2], "@", to_string(i));
                        SET_FILESCRIPT_PATH (lhs, rhs);

                        DEBUG_PRINT ("file script (%s <-- %s)", lhs.c_str(), rhs.c_str());
                    }
                }
            }
            else if (toked[0] == "REGISTER_DATAPATH")
            {
                if (range_start == -1 || range_end == -1)
                    SET_REGISTER_DATAPATH (toked[1], toked[2]);
                else
                {
                    for (uint32_t i = range_start; i <= range_end; i++)
                    {
                        string lhs = String::Replace (toked[1], "@", to_string(i));
                        string rhs = String::Replace (toked[2], "@", to_string(i));
                        SET_REGISTER_DATAPATH (lhs, rhs);

                        DEBUG_PRINT ("file register (%s <-- %s)", lhs.c_str(), rhs.c_str());
                    }
                }
            }
            else if (toked[0] == "CLOCK_PERIOD")
            {
                SET_CLOCK_PERIOD (toked[1], TO_SIM_TIMEUNIT (stof (toked[2])));
                DEBUG_PRINT ("clock period (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "UNIT_DYNAMIC_POWER")    
            {
                SET_UNIT_DYNAMIC_POWER (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("unit dynpower (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "UNIT_STATIC_POWER")    
            {
                SET_UNIT_STATIC_POWER (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("unit stapower (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "COMPONENT_DIS_POWER")    
            {
                SET_COMPONENT_DIS_POWER (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("component dispower (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "REGISTER_READ_ENERGY")    
            {
                SET_REGISTER_READ_ENERGY (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("register rdenergy (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "REGISTER_WRITE_ENERGY")    
            {
                SET_REGISTER_WRITE_ENERGY (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("register wrenergy (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "REGISTER_STATIC_POWER")    
            {
                SET_REGISTER_STATIC_POWER (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("reg stapower (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (toked[0] == "PARAMETER")
            {
                SET_PARAMETER (toked[1], stoi (toked[2]));
                DEBUG_PRINT ("parameter (%s <-- %s)", toked[1].c_str(), toked[2].c_str());
            }
            else if (line.length() > 1 && line[0] == '[')
            {
                vector<string> toked = String::Tokenize (line, "[fromto] ");
                range_start = stoi (toked[0]);
                range_end = stoi (toked[1]);
            }
            else
            {
                SIM_ERROR ("unknown spec name '%s' (regdata: %s, lineno: %u)",
                        GetName().c_str(), toked[0].c_str(), specfilename.c_str(), lineno);
                return false;
            }

            lineno++;
        }

        specfile.close ();
    }

    return true;
}


void Testbench::CreateComponentAndInitialize (PERMIT(Simulator))
{
    if (!creator)
        SYSTEM_ERROR ("creator cannot be null");

    TOP_COMPONENT = creator->Create ();

    Initialize (TRANSFER_KEY(Simulator));
}


string Testbench::GetStringParam (Testbench::ParamType ptype, string pname)
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

uint32_t Testbench::GetUIntParam (Testbench::ParamType ptype, string pname)
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
    else if (ptype == Testbench::COMPONENT_DIS_POWER)
    {
        if (compdispow.count (pname))
            return compdispow[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::REGISTER_READ_ENERGY)
    {
        if (regrdenergy.count (pname))
            return regrdenergy[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::REGISTER_WRITE_ENERGY)
    {
        if (regwrenergy.count (pname))
            return regwrenergy[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::REGISTER_STATIC_POWER)
    {
        if (regstapow.count (pname))
            return regstapow[pname];
        else
            return -1;
    }
    else if (ptype == Testbench::PARAMETER)
    {
        DEBUG_PRINT ("%d, %s", ptype, pname.c_str());
        if (modparams.count (pname))
            return modparams[pname];
        else
        {
            DESIGN_FATAL ("parameter '%s' has not been specified", 
                    GetName().c_str(), pname.c_str());
            return -1;
        }
    }
    else
        SYSTEM_ERROR ("parameter '%u' is not an integer", ptype);
}
