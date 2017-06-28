#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Module/Module.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <string>

using namespace std;


class AddOn: public Metadata
{
public:
    AddOn (const char *clsname, string iname)
        : Metadata (clsname, iname) {}

    Module* GetParent () { return parent; }
    void SetParent (Module *mod, PERMIT(Module)) { parent = mod; }

    string GetFullName ();

private:
    Module *parent;
};
