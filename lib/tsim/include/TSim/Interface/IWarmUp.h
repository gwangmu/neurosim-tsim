#pragma once

#include <TSim/Utility/AccessKey.h>

namespace TSim
{
    class IWarmUp
    {
    public:
        virtual void WarmUp (PERMIT(Simulator)) = 0;
    };
}
