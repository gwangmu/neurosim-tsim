#pragma once

#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/MetaRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;


class MetaFileRegister: public DenseFileRegister
{
public:
    MetaFileRegister (Type type, Attr attr)
        : DenseFileRegister ("MetaFileRegister", type, attr, 
                Prototype<MetaRegisterWord>::Get()) {}

    virtual RegisterWord* ParseRawString (string rawline)
    {
        std::string::size_type sz = 0;
        uint64_t ull = std::stoull (rawline, &sz, 0);

        return new MetaRegisterWord (ull);
    }
};
