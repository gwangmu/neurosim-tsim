#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;

struct ExampleRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    ExampleRegisterWord () 
        : RegisterWord ("ExampleRegisterWord"), value (0) {}

    ExampleRegisterWord (uint32_t value)
        : RegisterWord ("ExampleRegisterWord"), value (value) {}

    uint32_t value;
};
