#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Interface/INullable.h>
#include <string>

using namespace std;

struct Instruction: public Metadata, public INullable
{
public:
    Instruction (const char *clsname): Metadata (clsname, "") { null = false; }

    inline Instruction *
    inline bool IsNull () { return null; }

private:
    Instruction (): Metadata ("NullInstruction"
    bool null;
};

