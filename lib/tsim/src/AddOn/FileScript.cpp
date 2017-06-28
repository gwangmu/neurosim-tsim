#include <TSim/AddOn/FileScript.h>
#include <TSim/AddOn/Element/Instruction.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/String.h>

#include <string>
#include <vector>
#include <cinttypes>
#include <iostream>
#include <fstream>

using namespace std;


FileScript::FileScript (const char *clsname, string iname, Instruction *iproto):
    Script (clsname, iname, iproto)
{
    loaded = false;

    cursec = 0;
    nextinstr = 0;
    internal_cycle = 0;
}

bool FileScript::LoadFromFile (string filename)
{
    macrotask ("Reading script '%s'..", filename.c_str());

    ifstream scrfile;

    task ("open script file") 
    {
        scrfile.open (filename);
        if (!scrfile.is_open ()){
            DESIGN_ERROR ("cannot open script file '%s'",
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
                            else if (item[0] == "data")
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
                            else if (GetInstrPrototype()->GetClassName() != newinstr->GetClassName())
                            {
                                DESIGN_FATAL ("parser produced incompatible instruction (%s != %s)",
                                        GetName().c_str(), GetInstrPrototype()->GetClassName(), 
                                        newinstr->GetClassName());
                                return false;
                            }

                            sections.back().push_back (CycleInstrPair (instr_info.cycle, newinstr));

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

    loaded = true;
    PRINT ("total %u section(s), %u instruction(s)", total_nsec, total_ninstr);

    return true;
}


bool FileScript::NextSection ()
{
    if (cursec >= sections.size ())
        return false;

    internal_cycle = 0;
    nextinstr = 0;
    cursec++;

    return (cursec < sections.size());
}

Instruction* FileScript::NextInstruction ()
{
    Instruction *instr = nullptr;

    if (cursec < sections.size () &&
            nextinstr < sections[cursec].size ())
    {
        if (sections[cursec][nextinstr].cycle == internal_cycle)
        {
            instr = sections[cursec][nextinstr].instr;
            nextinstr++;
        }
    }

    internal_cycle++;
    return instr;
}


