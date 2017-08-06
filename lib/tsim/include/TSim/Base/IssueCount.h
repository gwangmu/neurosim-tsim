#pragma once
#include <cinttypes>

namespace TSim
{
    struct IssueCount
    {
        IssueCount () { error = 0; warning = 0; }
    
        uint32_t error;
        uint32_t warning;
    };
}
