#pragma once

#include <Base/IssueCount.h>
#include <Utility/AccessKey.h>

class Simulator;

class IValidatable
{
public:
    virtual IssueCount Validate (PERMIT(Simulator)) = 0;
};

