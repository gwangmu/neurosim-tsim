#pragma once

#include <TSim/AddOn/AddOn.h>
#include <TSim/Interface/ILoadable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>

using namespace std;

struct Instruction;
class Module;


class Script: public AddOn
{
public:
    Script (const char *clsname, Module *parent)
        : AddOn (clsname, parent) {}

    virtual bool NextSection () = 0;
    virtual Instruction* NextInstruction () = 0;

private:
    Module *parent;
};

