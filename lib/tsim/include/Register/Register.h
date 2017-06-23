#pragma once

#include <Base/Metadata.h>
#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

struct RegisterWord;


class Register: public Metadata
{
public:
    enum RegisterType { FF, SRAM, /* and more.. */ };

    struct RegisterAttr
    {
        RegisterAttr (uint32_t wordsize, uint32_t nwords)
        {
            this->wordsize = wordsize;
            this->nwords = nwords;
        }

        uint32_t wordsize;
        uint64_t nwords;
    };

public:
    Register (const char *clsname,
            string datatag, RegisterType type, 
            RegisterAttr attr, RegisterWord *wproto);

    RegisterWord* GetWordPrototype () { return wproto; }
    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawLine (string rawline) = 0;

    const RegisterWord* GetWord (uint64_t addr)
    {
        if (addr < words.size ())
            return words[addr];
        else
            return nullptr;
    }

private:
    string datatag;
    bool assigned;

    RegisterType type;
    RegisterAttr attr;
    RegisterWord *wproto;

    vector<RegisterWord *> words;
};
