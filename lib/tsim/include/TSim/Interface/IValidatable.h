#pragma once

#include <TSim/Base/IssueCount.h>
#include <TSim/Utility/AccessKey.h>

namespace TSim
{
    class Simulator;

    class IValidatable
    {
    public:
        virtual IssueCount Validate (PERMIT(Simulator)) = 0;
    };
}

