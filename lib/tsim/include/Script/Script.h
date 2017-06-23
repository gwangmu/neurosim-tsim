#pragma once

#include <Base/Metadata.h>
#include <string>

using namespace std;

struct Instruction;

class Script: public Metadata
{
public:
    Script (const char *clsname): Metadata (clsname, "") {}

    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    virtual bool NextSection () = 0;
    virtual Instruction* NextInstruction () = 0;

private:
    bool assigned;
};

