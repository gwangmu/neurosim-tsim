#pragma once

#include <TSim/Script/Instruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct ExampleInstruction: public Instruction
{
public:
    // NOTE: must define default constructor
    ExampleInstruction (): Instruction ("ExampleInstruction") {}

public:
    string data1;
    string data2;
    uint32_t data3;
};
