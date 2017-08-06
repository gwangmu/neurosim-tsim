#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

class NeuroSimTestbench;

class TimestepReporter: public Module
{
    VISIBLE_TO(NeuroSimTestbench);

public:
    TimestepReporter (string iname, Component *parent, uint32_t num_boards);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            Instruction *instr);

private:
    /* Port IDs */
    uint32_t IPORT_TSend;

    // internal state
    uint32_t num_boards;
    uint32_t end_counter;

    int cur_timestep;
};
