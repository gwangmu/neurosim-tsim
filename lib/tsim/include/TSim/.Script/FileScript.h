#pragma once

#include <TSim/AddOn/Script.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <vector>
#include <cinttypes>

using namespace std;

struct Instruction;

template <class INSTR_TYPE>
class FileScript: public Script, public ILoadable
{
private:
    struct CycleInstrPair
    {
        CycleInstrPair (uint64_t cycle, INSTR_TYPE instr):
            cycle (cycle), instr (instr) {}

        uint64_t cycle;
        INSTR_TYPE instr;
    };

public:
    FileScript (const char *clsname, Module *parent)
        : Script (clsname, parent) {}

    virtual bool NextSection () final;
    virtual const INSTR_TYPE& NextInstruction () final;

    bool LoadFromFile (string filename) final;
    bool IsLoaded () final;

    virtual INSTR_TYPE ParseRawString (string rawstr) = 0;

private:
    typedef vector<CycleInstrPair> ScriptSection;

    bool loaded;

    vector<ScriptSection> sections;
    uint32_t cursec;
    uint32_t nextinstr;
    uint32_t internal_cycle;
};
