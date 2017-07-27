#pragma once

#include <TSim/Utility/AccessKey.h>

class IWarmUp
{
public:
    virtual void WarmUp (PERMIT(Simulator)) = 0;
};
