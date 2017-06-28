#pragma once

#include <TSim/AddOn/FileRegister.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;

struct RegisterWord;
class Simulator;


class SparseFileRegister: public FileRegister
{
public:
    SparseFileRegister (const char *clsname, string iname, 
            Type type, Attr attr, RegisterWord *wproto);

    virtual RegisterWord* ParseRawString (string rawline) = 0;

    virtual const RegisterWord* GetWord (uint64_t addr) final;
    virtual bool SetWord (uint64_t addr, RegisterWord *word) final;

private:
    map<uint64_t, RegisterWord *> words;
};
