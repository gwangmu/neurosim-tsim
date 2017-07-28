#pragma once

#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/InputRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;


class EmptyFileRegister: public DenseFileRegister
{
public:
    EmptyFileRegister (Type type, Attr attr)
        : DenseFileRegister ("EmptyFileRegister", type, attr, 
                Prototype<InputRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        std::string::size_type sz = 0;
        uint64_t ull = std::stoull (rawline, &sz, 0);

        return new InputRegisterWord (ull);
    }
};
