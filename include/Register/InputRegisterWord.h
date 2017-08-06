#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;
using namespace TSim;

struct InputRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    InputRegisterWord () 
        : RegisterWord ("InputRegisterWord"), value (0) {}

    InputRegisterWord (uint64_t value)
        : RegisterWord ("InputRegisterWord"), value (value) {}

    uint64_t value;
};
