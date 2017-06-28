#pragma once

#include <TSim/AddOn/Element/Instruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct ExampleInstruction: public Instruction
{
public:
    // NOTE: must define default constructor w/ initialization
    ExampleInstruction (): Instruction ("ExampleInstruction") 
    {
        data1 = "";
        data2 = "";
        data3 = 0;
    }

public:
    string data1;
    string data2;
    uint32_t data3;
};
