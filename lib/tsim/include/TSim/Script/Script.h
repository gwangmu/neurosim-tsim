#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Interface/IValidatable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>

using namespace std;

struct Instruction;
class Module;

class Script: public Metadata
{
public:
    Script (const char *clsname): Metadata (clsname, "") {}

    Module* GetParent () { return parent; }
    void SetParent (Module *module, PERMIT(Module)) { parent = module; }

    virtual bool NextSection () = 0;
    virtual Instruction* NextInstruction () = 0;

private:
    Module *parent;
};

