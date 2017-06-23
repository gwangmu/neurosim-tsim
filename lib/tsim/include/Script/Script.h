#pragma once

#include <Base/Metadata.h>
#include <string>

using namespace std;

struct Instruction;

class Script: public Metadata
{
public:
    Script (const char *clsname, string name):
        Metadata (clsname, name) {}

    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    virtual Instruction* NextInstruction () = 0;

private:
    bool assigned;
};

