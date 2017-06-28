#pragma once

#include <TSim/AddOn/FileRegister.h>
#include <TSim/AddOn/Element/RegisterWord.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

class Simulator;


class DenseFileRegister: public FileRegister
{
public:
    DenseFileRegister (const char *clsname, string iname, 
            Type type, Attr attr, RegisterWord *wproto);

    virtual RegisterWord* ParseRawString (string rawline) = 0;

    virtual const RegisterWord* GetWord (uint64_t addr) final;
    virtual bool SetWord (uint64_t addr, RegisterWord *word) final;

private:
    vector<RegisterWord *> words;
};
