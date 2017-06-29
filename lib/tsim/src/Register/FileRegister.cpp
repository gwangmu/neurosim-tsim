#include <TSim/Register/FileRegister.h>
#include <TSim/Register/RegisterWord.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <vector>
#include <cinttypes>
#include <iostream>
#include <fstream>

using namespace std;

struct RegisterWord;


FileRegister::FileRegister (const char *clsname, Type type, Attr attr, RegisterWord *wproto)
    : Register (clsname, type, attr, wproto)
{
    loaded = false;
}


bool FileRegister::LoadDataFromFile (string filename)
{
    macrotask ("Reading register data '%s'..", filename.c_str());

    ifstream regfile;

    task ("open register data file") 
    {
        regfile.open (filename);
        if (!regfile.is_open ()){
            DESIGN_ERROR ("cannot open register data file '%s'",
                    "FileRegister", filename.c_str());
            return false;
        }
    }

    uint64_t total_nword = 0;

    task ("parse/load register data file") 
    {
        enum { 
            FINDING_CLASSNAME, FINDING_DATA_LABEL, READING_DATA
        } state = FINDING_CLASSNAME;

        uint32_t lineno = 1;
        string line;

        struct { uint64_t addr = -1; string data = ""; } word_info;

        while (getline (regfile, line))
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
                                SIM_ERROR ("incompatible data '%s'",
                                        GetName().c_str(), filename.c_str());
                                return false;
                            }

                            state = FINDING_DATA_LABEL;
                        }
                        else
                        {
                            SIM_ERROR ("expected 'CLASSNAME' (regdata: %s, lineno: %u)",
                                    GetName().c_str(), filename.c_str(), lineno);
                            return false;
                        }
                    } break;
                case FINDING_DATA_LABEL:
                    {
                        vector<string> toked = String::Tokenize (line, ":");
                        if (toked[0] == "DATA")
                            state = READING_DATA;
                        else
                        {
                            SIM_ERROR ("expected 'DATA' (regdata: %s, lineno: %u)",
                                    GetName().c_str(), filename.c_str(), lineno);
                            return false;
                        }
                    } break;
                case READING_DATA:
                    {
                        vector<string> toked = String::Tokenize (line, ",");
                        for (int i = 0; i < toked.size(); i++)
                        {
                            vector<string> item = String::Tokenize (toked[i], "=");

                            if (item[0] == "addr")
                                word_info.addr = stoi (item[1]);
                            else if (item[0] == "data")
                                word_info.data = item[1];
                            else
                            {
                                SIM_ERROR ("unknown register word property '%s' (regdata: %s, line: %u)",
                                        GetName().c_str(), item[0].c_str(), filename.c_str(), lineno);
                                return false;
                            }
                        }

                        RegisterWord *newword = ParseRawString (word_info.data);
                        if (!newword)
                        {
                            SIM_ERROR ("invalid register word data (regdata: %s, line: %u)",
                                    GetName().c_str(), filename.c_str(), lineno);
                            return false;
                        }
                        else if (GetWordPrototype()->GetClassName() != newword->GetClassName())
                        {
                            DESIGN_FATAL ("parser produced incompatible register word (%s != %s)",
                                    GetName().c_str(), GetWordPrototype()->GetClassName(), 
                                    newword->GetClassName());
                            return false;
                        }

                        if (!SetWord (word_info.addr, newword))
                            SIM_ERROR ("failed to register word at 0x%lx", 
                                    GetName().c_str(), word_info.addr);

                        total_nword++;
                    } break;
                default:
                    {
                        SYSTEM_ERROR ("bogus register data reading state");
                    } break;
            }

            lineno++;
        }

        if (state != READING_DATA)
        {
            SIM_WARNING ("register data ('%s') incompletely ended",
                    GetName().c_str(), filename.c_str());
        }

        regfile.close ();
    }

    loaded = true;
    PRINT ("total %lu word(s)", total_nword);

    return true;
}

IssueCount FileRegister::Validate (PERMIT(Simulator))
{
    IssueCount icount;

    if (!loaded)
    {
        SIM_ERROR ("register file not loaded", GetName().c_str());
        icount.error++;
    }

    return icount;
}
