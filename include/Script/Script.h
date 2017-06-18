#pragma once

#include <Base/Metadata.h>
#include <string>

using namespace std;

struct Instruction;

class Script: public Metadata
{
public:
    Script (string name);

    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    string GetName ();
    virtual Instruction* NextInstruction () = 0;

private:
    string name;
    bool assigned;
};

