#pragma once

#include <TSim/Register/SparseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/DramRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;


class DramFileRegister: public SparseFileRegister
{
public:
    DramFileRegister (Type type, Attr attr)
        : SparseFileRegister ("DramFileRegister", type, attr, 
                Prototype<DramRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        std::string::size_type sz = 0;
        uint64_t ull = std::stoull (rawline, &sz, 0);

        return new DramRegisterWord (ull);
    }
};
