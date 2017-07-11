#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;

struct DramRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    DramRegisterWord () 
        : RegisterWord ("DramRegisterWord"), value (0) {}

    DramRegisterWord (uint64_t value)
        : RegisterWord ("DramRegisterWord"), value (value) {}

    uint64_t value;
};
