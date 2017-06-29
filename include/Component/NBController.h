#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>
#include <Message/StateMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class NBController: public Module
{
public:
    NBController (string iname, Component *parent, uint32_t max_index);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_State;
    uint32_t IPORT_DeltaG;
    uint32_t IPORT_Tsparity;

    // Output port
    uint32_t OPORT_SRAM; // Connect to SRAM
    uint32_t OPORT_NB; // Connect to Neuron block
    uint32_t OPORT_End;
    
    // Internal states
    uint32_t max_idx_;
    uint32_t idx_counter_;
    bool ts_parity_;
    bool deltaG_ready, state_ready;
    bool is_idle;

    /* Internal Registers */
    uint64_t deltaG_reg;
    State state_reg;
};
