#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

struct RegisterWord;
class Module;


class Register: public Metadata
{
public:
    enum Type { FF, SRAM, /* and more.. */ };

    struct Attr
    {
        Attr (uint32_t wordsize, uint32_t nwords)
        {
            this->wordsize = wordsize;
            this->nwords = nwords;
        }

        uint32_t wordsize;
        uint64_t nwords;
    };

public:
    Register (const char *clsname, Type type, 
            Attr attr, RegisterWord *wproto);

    RegisterWord* GetWordPrototype () { return wproto; }
    Module* GetParent () { return parent; }
    void SetParent (Module *module, PERMIT(Module)) { parent = module; }

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawString (string rawline) = 0;

    const RegisterWord* GetWord (uint64_t addr);

private:
    Module *parent;

    Type type;
    Attr attr;
    RegisterWord *wproto;

    vector<RegisterWord *> words;
};
