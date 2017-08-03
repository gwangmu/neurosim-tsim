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
    bool is_inh;
    uint32_t spk_len;
    std::list<uint32_t> spike_idx;
};
