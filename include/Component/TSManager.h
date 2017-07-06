#pragma once


#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class NeuroSimTestbench;

class TSManager: public Module
{
    VISIBLE_TO(NeuroSimTestbench);

public:
    TSManager (string iname, Component *parent, uint32_t num_boards);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_DynFin, IPORT_idle;
    uint32_t IPORT_TSend;

    // Output port
    uint32_t OPORT_TSParity, OPORT_TSend;

    // internal state
    bool dyn_fin, prop_idle;
    bool ts_parity;
    bool is_finish;

    uint32_t num_boards;
    uint32_t end_counter;

    int cur_timestep;
};
