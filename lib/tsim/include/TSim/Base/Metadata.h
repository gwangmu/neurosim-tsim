#pragma once

#include <TSim/Utility/Logging.h>
#include <string>

using namespace std;

namespace TSim
{
    class Metadata
    {
    public:
        static inline string SEPARATOR () { return "."; }
        static inline string SCOPE () { return "::"; }
    
        Metadata (const char *clsname, string iname)
        {
            if (!clsname)
                DESIGN_FATAL ("clsname cannot be null", iname.c_str());
            this->clsname = clsname;
    
            if (iname.empty ())
                this->iname = "(noname)";
            else
            {
                if (iname.find(SEPARATOR()) != string::npos)
                    DESIGN_FATAL ("instance name cannot contain '%s'",
                            (string (clsname) + " " + iname).c_str(), SEPARATOR().c_str());
                this->iname = iname;
            }
        }
    
        virtual string GetInstanceName () { return iname; }
        const char* GetClassName () { return clsname; }
        string GetName () { return string(clsname) + " " + GetInstanceName(); }
    
        virtual string GetSummary () { return ""; }
    
    private:
        const char* clsname;
        string iname;
    };
}
