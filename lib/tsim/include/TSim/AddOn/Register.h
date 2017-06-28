#pragma once

#include <TSim/AddOn/AddOn.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <type_traits>

using namespace std;

struct RegisterWord;
class Module;


class Register: public AddOn
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
    Register (const char *clsname, string iname, 
            Type type, Attr attr, RegisterWord *wproto);

    inline const Type& GetType () { return type; }
    inline const Attr& GetAttr () { return attr; }
    inline RegisterWord* GetWordPrototype () { return wproto; }

    virtual const RegisterWord* GetWord (uint64_t addr) = 0;
    virtual bool SetWord (uint64_t addr, RegisterWord word) = 0;

private:
    Type type;
    Attr attr;
    RegisterWord *wproto;
};
