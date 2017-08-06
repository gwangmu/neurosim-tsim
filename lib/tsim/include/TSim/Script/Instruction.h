#pragma once

#include <TSim/Base/Metadata.h>
#include <string>

using namespace std;

namespace TSim
{
    struct Instruction: public Metadata
    {
    public:
        Instruction (const char *clsname): Metadata (clsname, "") {}
    };
}

