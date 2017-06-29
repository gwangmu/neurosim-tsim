#pragma once

#include <TSim/Register/FileRegister.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;

struct RegisterWord;
class Simulator;


class DenseFileRegister: public FileRegister
{
public:
    DenseFileRegister (const char *clsname, Type type, 
            Attr attr, RegisterWord *wproto);

    virtual RegisterWord* ParseRawString (string rawline) = 0;

    virtual const RegisterWord* GetWord (uint64_t addr) final;
    virtual bool SetWord (uint64_t addr, RegisterWord *word) final;

private:
    vector<RegisterWord *> words;
};
