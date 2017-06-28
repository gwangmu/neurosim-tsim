#pragma once

#include <TSim/AddOn/Register.h>
#include <TSim/Interface/ILoadable.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/String.h>

#include <string>
#include <vector>
#include <cinttypes>
#include <iostream>
#include <fstream>

using namespace std;

struct RegisterWord;


class FileRegister: public Register, public ILoadable
{
public:
    FileRegister (const char *clsname, string iname, 
            Type type, Attr attr, RegisterWord *wproto);

    virtual bool LoadFromFile (string filename) final;
    virtual bool IsLoaded () { return loaded; }

    virtual RegisterWord* ParseRawString (string rawline) = 0;

    virtual const RegisterWord* GetWord (uint64_t addr) = 0;
    virtual bool SetWord (uint64_t addr, RegisterWord *word) = 0;

private:
    bool loaded;
};
