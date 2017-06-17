#pragma once

struct IssueCount
{
    IssueCount () { error = 0; warning = 0; }

    uint32_t error;
    uint32_t warning;
};

class IValidatable
{
public:
    virtual IssueCount Validate (PERMIT(Simulator)) = 0;
};
