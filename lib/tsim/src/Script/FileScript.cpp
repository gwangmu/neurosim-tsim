#include <Script/FileScript.h>
#include <Script/Instruction.h>
#include <Utility/Logging.h>
#include <Utility/String.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


FileScript::FileScript (const char *clsname, Instruction *iproto):
    Script (clsname)
{
    cursec = 0;
    curinstr = 0;

    if (!iproto)
        DESIGN_FATAL ("null instructino prototype", GetName().c_str ());
    this->iproto = iproto;
}

bool FileScript::LoadScriptFromFile (string filename)
{
    macrotask ("Reading script '%s'..", filename.c_str());

    ifstream scrfile;

    task ("open script file") 
    {
        scrfile.open (filename);
        if (!scrfile.is_open ()){
            DESIGN_ERROR ("cannot open a script file '%s'",
                    "FileScript", filename.c_str());
            return false;
        }
    }
    
    uint32_t total_nsec = 0;
    uint32_t total_ninstr = 0;

    task ("parse/load script file") 
    {
        enum { 
            FINDING_CLASSNAME, FINDING_SCRIPT, 
            FINDING_SECTION, READING_INSTRUCTIONS 
        } state = FINDING_CLASSNAME;

        uint32_t lineno = 1;
        string line;

        uint32_t secno = 0;
        uint32_t instrno = 0;
        struct { uint32_t cycle = -1; string data = ""; } instr_info, prev_instr_info;

        while (getline (scrfile, line))
        {
            line = String::Trim (line);

            if (line[0] == '#') continue;
            if (line == "") continue;

            switch (state)
            {
                case FINDING_CLASSNAME:
                {
                    vector<string> toked = String::Tokenize (line, ":");
                    if (toked[0] == "CLASSNAME")
                    {
                        if (toked[1] != GetClassName())
                        {
                            SIM_ERROR ("incompatible script '%s'",
                                    GetName().c_str(), filename.c_str());
                            return false;
                        }
                        
                        state = FINDING_SCRIPT;
                    }
                    else
                    {
                        SIM_ERROR ("expected 'CLASSNAME' (script: %s, lineno: %u)",
                                GetName().c_str(), filename.c_str(), lineno);
                        return false;
                    }
                } break;
                case FINDING_SCRIPT:
                {
                    vector<string> toked = String::Tokenize (line, ":");
                    if (toked[0] == "SCRIPT")
                        state = FINDING_SECTION;
                    else
                    {
                        SIM_ERROR ("expected 'SCRIPT' (script: %s, lineno: %u)",
                                GetName().c_str(), filename.c_str(), lineno);
                        return false;
                    }
                } break;
                case FINDING_SECTION:
                {
                    if (line == "section")
                    {
                        secno++;
                        instrno = 1;
                        sections.push_back (ScriptSection());
                        state = READING_INSTRUCTIONS;
                    }
                    else
                    {
                        SIM_ERROR ("expected 'section' (script: %s, lineno: %u)",
                                GetName().c_str(), filename.c_str(), lineno);
                        return false;
                    }
                } break;
                case READING_INSTRUCTIONS:
                {
                    if (instrno == 0)
                    {
                        SYSTEM_ERROR ("uninitialized instruction no. (class: %s)",
                                GetName().c_str());
                        return false;
                    }

                    if (secno == 0)
                    {
                        SYSTEM_ERROR ("uninitialized section no. (class: %s)",
                                GetName().c_str());
                        return false;
                    }

                    if (line == "endsection") 
                    {
                        total_nsec++;
                        state = FINDING_SECTION;
                    }
                    else
                    {
                        vector<string> toked = String::Tokenize (line, ",");
                        for (int i = 0; i < toked.size(); i++)
                        {
                            vector<string> item = String::Tokenize (toked[i], "=");

                            if (item[0] == "cycle")
                                instr_info.cycle = stoi (item[1]);
                            else if (item[1] == "data")
                                instr_info.data = item[1];
                            else
                            {
                                SIM_ERROR ("unknown instruction property '%s' (script: %s, line: %u)",
                                        GetName().c_str(), item[0].c_str(), filename.c_str(), lineno);
                                return false;
                            }
                        }
                        
                        if (instr_info.cycle == -1)
                        {
                            SIM_ERROR ("instruction with no cycle (script: %s, line: %u)",
                                    GetName().c_str(), filename.c_str(), lineno);
                            return false;
                        }
                        else if (prev_instr_info.cycle != -1 && 
                                prev_instr_info.cycle >= instr_info.cycle)
                        {
                            SIM_ERROR ("inverted cycle order (%u >= %u)",
                                    GetName().c_str(), prev_instr_info.cycle, instr_info.cycle);
                            return false;
                        }
                        else
                        {
                            Instruction *newinstr = ParseRawString (instr_info.data);
                            if (!newinstr)
                            {
                                SIM_ERROR ("invalid instruction data (stript: %s, line: %u)",
                                        GetName().c_str(), filename.c_str(), lineno);
                                return false;
                            }
                            else if (iproto->GetClassName() != newinstr->GetClassName())
                            {
                                DESIGN_FATAL ("parser produced incompatible instruction (%s != %s)",
                                        GetName().c_str(), iproto->GetClassName(), 
                                        newinstr->GetClassName());
                                return false;
                            }

                            sections.back().push_back (newinstr);

                            prev_instr_info = instr_info;

                            instr_info.cycle = -1;
                            instr_info.data = "";
                            instrno++;
                            total_ninstr++;
                        }
                    }                
                } break;
                default:
                {
                    SYSTEM_ERROR ("bogus script reading state");
                } break;
            }

            lineno++;
        }

        if (state != FINDING_SECTION)
        {
            SIM_WARNING ("script ('%s') incompletely ended",
                    GetName().c_str(), filename.c_str());
        }

        scrfile.close ();
    }

    PRINT ("total %u section(s), %u instruction(s)", total_nsec, total_ninstr);

    return true;
}
