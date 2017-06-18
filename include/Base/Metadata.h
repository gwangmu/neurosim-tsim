#pragma once

#include <Utility/Logging.h>
#include <string>

using namespace std;

class Metadata
{
public:
    Metadata (const char *clsname, string iname)
    {
        if (!clsname)
            DESIGN_FATAL ("clsname cannot be null", name.c_str());
        this->clsname = clsname;

        if (iname.empty ())
            this->iname = "(noname)";
        else
            this->iname = iname;
    }

    virtual string GetInstanceName () { return iname; }
    const char* GetClassName () { return clsname; }
    string GetName () { return string(clsname) + " " + GetInstanceName(); }

    virtual string GetSummary () {}

private:
    const char* clsname;
    string iname;
};
