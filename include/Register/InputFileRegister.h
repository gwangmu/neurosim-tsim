#pragma once

#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/InputRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;
using namespace TSim;


class InputFileRegister: public DenseFileRegister
{
public:
    InputFileRegister (Type type, Attr attr)
        : DenseFileRegister ("InputFileRegister", type, attr, 
                Prototype<InputRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        std::string::size_type sz = 0;
        uint64_t ull = std::stoull (rawline, &sz, 0);

        return new InputRegisterWord (ull);
    }
};
