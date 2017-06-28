#pragma once

#include <TSim/Register/Register.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <cinttypes>

using namespace std;

struct RegisterWord;
class Simulator;


class FileRegister: public Register, public IValidatable
{
public:
    FileRegister (const char *clsname, Type type, 
            Attr attr, RegisterWord *wproto);

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawString (string rawline) = 0;
    virtual IssueCount Validate (PERMIT(Simulator)) final;

    // TODO need to be optimized
    virtual const RegisterWord* GetWord (uint64_t addr) = 0;
    virtual bool SetWord (uint64_t addr, RegisterWord *word) = 0;

private:
    bool loaded;
};
