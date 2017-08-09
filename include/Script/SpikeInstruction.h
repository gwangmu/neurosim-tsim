#pragma once

#include <TSim/Script/Instruction.h>

#include <cinttypes>
#include <string>
#include <list>

using namespace std;
using namespace TSim;

struct SpikeInstruction: public Instruction
{
public:
    // NOTE: must define default constructor
    SpikeInstruction (): Instruction ("SpikeInstruction") {}

public:
    enum SpikeType {EXC, INH, DELAYED};
    SpikeType type;
    uint32_t spk_len;
    std::list<uint32_t> spike_idx;
};
