#pragma once

#include <Base/IssueCount.h>

class IssueCounter
{
public:
    void CheckError () { icount.error++; }
    void CheckWarning () { icount.warning++; }

    IssueCount GetIssueCount () { return icount; }

private:
    IssueCount icount;
};
