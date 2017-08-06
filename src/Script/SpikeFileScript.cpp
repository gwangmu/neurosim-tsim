#include <TSim/Utility/String.h>
#include <TSim/Utility/Logging.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>


#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


Instruction* SpikeFileScript::ParseRawString (string rawstr)
{
    vector<string> toked = String::Tokenize (rawstr, "/");
    
    SpikeInstruction *instr = new SpikeInstruction ();

    instr->is_inh = stoi(toked[0]);
    for(int i=1; i<toked.size(); i++)
    {
        instr->spike_idx.push_back(stoi(toked[i]));
    }

    return instr;
}
