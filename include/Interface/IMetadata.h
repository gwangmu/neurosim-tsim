#pragma once

class IMetadata
{
public:
    virtual string GetName () = 0;
    virtual string GetFullName () = 0;
    virtual const char* GetClassName () = 0;
    virtual string GetSummary () = 0;
};
