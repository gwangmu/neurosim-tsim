#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;

struct MetaRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    MetaRegisterWord () 
        : RegisterWord ("MetaRegisterWord"), value (0) {}

    MetaRegisterWord (uint64_t value)
        : RegisterWord ("MetaRegisterWord"), value (value) {}

    uint64_t value;
};
