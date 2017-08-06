#pragma once

#include <TSim/Register/SparseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/DelayRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;
using namespace TSim;


class DelayFileRegister: public SparseFileRegister
{
public:
    DelayFileRegister (Type type, Attr attr)
        : SparseFileRegister ("DelayFileRegister", type, attr, 
                Prototype<DelayRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        std::string::size_type sz = 0;
        uint64_t ull = std::stoull (rawline, &sz, 0);

        return new DelayRegisterWord (ull, 0, 0);
    }
};
