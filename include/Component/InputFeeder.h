#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class InputFeeder: public Module
{
public:
    InputFeeder (string iname, Component *parent, 
            int num_propagators);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            Instruction *instr);

private:
    // Port IDs
    std::vector<uint32_t> PORT_axons;
    uint32_t PORT_idle, PORT_TSparity;

    // Internal states
    bool is_idle_;
    int num_propagators_;
    uint64_t dram_size_;
    uint32_t num_inputs_, num_input_neurons_;
    uint32_t counter_;
    bool ts_parity_;
};
