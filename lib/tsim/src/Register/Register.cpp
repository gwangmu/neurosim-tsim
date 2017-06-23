#include <Register/Register.h>
#include <Register/RegisterWord.h>
#include <Utility/Logging.h>
#include <Utility/String.h>

#include <string>
#include <vector>
#include <cinttypes>
#include <iostream>
#include <fstream>

using namespace std;


Register::Register (const char *clsname, Register::Type type, 
        Register::Attr attr, RegisterWord *wproto):
    Metadata (clsname, ""), attr (attr)
{
    if (attr.wordsize == 0 || attr.wordsize > 1024)
        DESIGN_ERROR ("inappropriate word size (%u > 1024 or %u == 0)",
                GetName().c_str(), attr.wordsize, attr.wordsize);

    if (!wproto)
        DESIGN_FATAL ("null register word prototype", GetName().c_str());
    this->wproto = wproto;
}

bool Register::LoadDataFromFile (string filename)
{
    macrotask ("Reading register data '%s'..", filename.c_str());

    ifstream regfile;

    task ("open register data file") 
    {
        regfile.open (filename);
        if (!regfile.is_open ()){
            DESIGN_ERROR ("cannot open a script file '%s'",
                    "FileScript", filename.c_str());
            return false;
        }
    }
    
    uint64_t total_nword = 0;
    uint64_t max_addr = 0;

    task ("parse/load register data file") 
    {
        enum { 
            FINDING_CLASSNAME, FINDING_DATA_LABEL, READING_DATA
        } state = FINDING_CLASSNAME;

        uint32_t lineno = 1;
        string line;

        struct { uint64_t addr = -1; string data = ""; } word_info, prev_word_info;

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
                        else if (item[1] == "data")
                            word_info.data = item[1];
                        else
                        {
                            SIM_ERROR ("unknown register word property '%s' (regdata: %s, line: %u)",
                                    GetName().c_str(), item[0].c_str(), filename.c_str(), lineno);
                            return false;
                        }
                    }
                    
                    if (word_info.addr != -1 && prev_word_info.addr >= word_info.addr)
                    {
                        SIM_ERROR ("inverted address order (%lx > %lx)",
                                GetName().c_str(), prev_word_info.addr,
                                word_info.addr);
                        return false;
                    }
                    else
                    {
                        RegisterWord *newword = ParseRawString (word_info.data);
                        if (!newword)
                        {
                            SIM_ERROR ("invalid register word data (regdata: %s, line: %u)",
                                    GetName().c_str(), filename.c_str(), lineno);
                            return false;
                        }
                        else if (wproto->GetClassName() != newword->GetClassName())
                        {
                            DESIGN_FATAL ("parser produced incompatible register word (%s != %s)",
                                    GetName().c_str(), wproto->GetClassName(), 
                                    newword->GetClassName());
                            return false;
                        }

                        words.push_back (newword);
                        max_addr = word_info.addr;
                        total_nword++;
                    }
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

    PRINT ("total %lu word(s), max address %lx", total_nword, max_addr);

    return true;
}

const RegisterWord* Register::GetWord (uint64_t addr)
{
    if (addr < words.size ())
        return words[addr];
    else
    {
        SIM_WARNING ("accessing out-of-range address (%lx)", 
                GetName().c_str(), addr);
        return nullptr;
    }
}
