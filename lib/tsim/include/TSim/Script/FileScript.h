#pragma once

#include <TSim/Script/Script.h>
#include <TSim/Interface/IValidatable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <vector>
#include <cinttypes>

using namespace std;

namespace TSim
{
    struct Instruction;

    class FileScript: public Script, public IValidatable
    {
    private:
        struct CycleInstrPair
        {
            CycleInstrPair (uint64_t cycle, Instruction *instr):
                cycle (cycle), instr (instr) {}
    
            uint64_t cycle;
            Instruction *instr;
        };
    
    public:
        FileScript (const char *clsname, Instruction *iproto);
    
        virtual bool NextSection ();
        virtual Instruction* NextInstruction ();
    
        bool LoadScriptFromFile (string filename);
        virtual Instruction* ParseRawString (string rawstr) = 0;
    
        virtual IssueCount Validate (PERMIT(Simulator)) final;
    
    private:
        typedef vector<CycleInstrPair> ScriptSection;
    
        bool loaded;
    
        Instruction *iproto;
        vector<ScriptSection> sections;
        uint32_t cursec;
        uint32_t nextinstr;
        uint32_t internal_cycle;
    };
}
