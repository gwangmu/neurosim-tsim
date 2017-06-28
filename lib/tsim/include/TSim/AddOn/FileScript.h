#pragma once

#include <TSim/AddOn/Script.h>
#include <TSim/Interface/ILoadable.h>
#include <TSim/AddOn/Element/Instruction.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/String.h>

#include <string>
#include <vector>
#include <cinttypes>
#include <iostream>
#include <fstream>

using namespace std;


class FileScript: public Script, public ILoadable
{
private:
    struct CycleInstrPair
    {
        CycleInstrPair (uint64_t cycle, Instruction* instr):
            cycle (cycle), instr (instr) {}

        uint64_t cycle;
        Instruction *instr;
    };

public:
    FileScript (const char *clsname, string iname, Instruction *iproto);

    virtual bool LoadFromFile (string filename) final;
    virtual bool IsLoaded () final { return loaded; }

    virtual Instruction* ParseRawString (string rawstr) = 0;

    virtual bool NextSection () final;
    virtual Instruction* NextInstruction () final;

private:
    typedef vector<CycleInstrPair> ScriptSection;

    bool loaded;

    vector<ScriptSection> sections;
    uint32_t cursec;
    uint32_t nextinstr;
    uint32_t internal_cycle;
};
