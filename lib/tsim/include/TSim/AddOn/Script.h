#pragma once

#include <TSim/AddOn/AddOn.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <string>
#include <type_traits>

using namespace std;

struct Instruction;
class Module;


class Script: public AddOn
{
public:
    Script (const char *clsname, string iname, Instruction *iproto)
        : AddOn (clsname, iname)
    {
        if (!iproto)
            DESIGN_FATAL ("null instruction prototype", GetName().c_str());
        this->iproto = iproto;
    }

    inline Instruction* GetInstrPrototype () { return iproto; }

    virtual bool NextSection () = 0;
    virtual Instruction* NextInstruction () = 0;

private:
    Instruction *iproto;
};
