#include <TSim/Utility/String.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Instruction* SpikeFileScript::ParseRawString (string rawstr)
{
    vector<string> toked = String::Tokenize (rawstr, ",");
    
    SpikeInstruction *instr = new SpikeInstruction ();

    for(int i=0; i<toked.size(); i++)
    {
        instr->spike_idx.push_back(stoi(toked[i]));
    }

    return instr;
}
