#pragma once

#include <TSim/Base/Metadata.h>
#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

struct RegisterWord;


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
    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawString (string rawline) = 0;

    const RegisterWord* GetWord (uint64_t addr);

private:
    bool assigned;

    Type type;
    Attr attr;
    RegisterWord *wproto;

    vector<RegisterWord *> words;
};
