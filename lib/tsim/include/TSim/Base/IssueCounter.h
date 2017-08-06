#pragma once

#include <TSim/Base/IssueCount.h>

namespace TSim
{
    class IssueCounter
    {
    public:
        void CheckError () { icount.error++; }
        void CheckWarning () { icount.warning++; }
    
        IssueCount GetIssueCount () { return icount; }
    
    private:
        IssueCount icount;
    };
}
