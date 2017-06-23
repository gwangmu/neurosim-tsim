#pragma once

#include <Script/Script.h>
#include <string>
#include <vector>
#include <cinttypes>

using namespace std;

struct Instruction;

class FileScript: public Script
{
public:
    FileScript (const char *clsname, Instruction *iproto);

    bool LoadScriptFromFile (string filename);
    virtual Instruction* ParseRawString (string rawstr) = 0;

private:
    typedef vector<Instruction *> ScriptSection;

    Instruction *iproto;
    vector<ScriptSection> sections;
    uint32_t cursec;
    uint32_t curinstr;
};
