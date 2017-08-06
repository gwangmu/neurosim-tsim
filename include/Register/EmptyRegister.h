#pragma once

#include <TSim/Register/Register.h>
#include <TSim/Utility/Prototype.h>
#include <Register/InputRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;
using namespace TSim;


class EmptyRegister: public Register
{
public:
    EmptyRegister (Type type, Attr attr)
        : Register ("EmptyRegister", type, attr, 
                Prototype<InputRegisterWord>::Get()) {}

    virtual const RegisterWord* GetWord (uint64_t addr) 
    { 
        IncrReadCount();
        return nullptr;
    }

    virtual bool SetWord (uint64_t addr, RegisterWord *word) 
    { 
        IncrWriteCount();
        delete word;
        return true; 
    }

protected:
    virtual bool InitWord (uint64_t addr, RegisterWord *word) { return true; }
};
