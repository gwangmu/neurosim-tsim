#include <TSim/Utility/String.h>

#include <Script/ExampleFileScript.h>
#include <Script/ExampleInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Instruction* ExampleFileScript::ParseRawString (string rawstr)
{
    vector<string> toked = String::Tokenize (rawstr, "/");
    
    ExampleInstruction *instr = new ExampleInstruction ();
    instr->data1 = toked[0];
    instr->data2 = toked[1];
    instr->data3 = stoi(toked[2]);

    return instr;
}
