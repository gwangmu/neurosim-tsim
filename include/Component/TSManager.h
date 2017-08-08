#pragma once


#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

class NeuroSimTestbench;

class TSManager: public Module
{
    VISIBLE_TO(NeuroSimTestbench);

public:
    TSManager (string iname, Component *parent, uint32_t num_boards);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_DynFin, IPORT_AccIdle, IPORT_idle;
    uint32_t IPORT_TSend;

    // Output port
    uint32_t OPORT_TSParity, OPORT_TSend;

    // internal state
    bool dyn_fin, prop_idle, acc_idle;
    bool ts_parity;
    bool is_finish, is_start, is_end;

    uint32_t num_boards;
    uint32_t end_counter;

    uint64_t stack_counter[8];

    int cur_timestep;
    int ts_buf_;
};
