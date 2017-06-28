#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Interface/INullable.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <cinttypes>

using namespace std;

struct RegisterWord;
class Module;
class Simulator;


class Register: public Metadata, public INullable
{
public:
    enum Type { FF, SRAM };

    struct Attr
    {
        Attr (uint32_t wordsize, uint32_t addrsize)
        {
            this->wordsize = wordsize;
            this->addrsize = addrsize;
        }

        uint32_t wordsize;
        uint64_t addrsize;
    };

public:
    Register (const char *clsname, Type type, 
            Attr attr, RegisterWord *wproto);

    Module* GetParenit () { return parent; }
    void SetParent (Module *module, PERMIT(Module)) { parent = module; }

    virtual const RegisterWord* GetWord (uint64_t addr) = 0;

private:
    Module *parent;

    Type type;
    Attr attr;
    RegisterWord *wproto;
};
