#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;

struct EmptyRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    EmptyRegisterWord () 
        : RegisterWord ("EmptyRegisterWord"), value (0) {}

    EmptyRegisterWord (uint64_t value)
        : RegisterWord ("EmptyRegisterWord"), value (value) {}

    uint64_t value;
};
