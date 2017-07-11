#pragma once

#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/ExampleRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;


class ExampleFileRegister: public DenseFileRegister
{
public:
    ExampleFileRegister (Type type, Attr attr)
        : DenseFileRegister ("ExampleFileRegister", type, attr, 
                Prototype<ExampleRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        return new ExampleRegisterWord (stoi(rawline));
    }
};
