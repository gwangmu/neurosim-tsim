#pragma once

#include <TSim/Script/Instruction.h>

#include <cinttypes>
#include <string>
#include <list>

using namespace std;

struct SpikeInstruction: public Instruction
{
public:
    // NOTE: must define default constructor
    SpikeInstruction (): Instruction ("SpikeInstruction") {}

public:
    std::list<uint32_t> spike_idx;
};
