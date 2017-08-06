#pragma once

#include <TSim/Base/Metadata.h>
#include <string>

using namespace std;

namespace TSim
{
    struct RegisterWord: public Metadata
    {
    public:
        RegisterWord (const char *clsname): Metadata (clsname, "") {}
    };
}

