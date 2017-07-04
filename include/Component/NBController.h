#pragma once

#include <TSim/Module/Module.h>
#include <Message/StateMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class NBController: public Module
{
public:
    NBController (string iname, Component *parent, uint32_t max_index);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_State;
    uint32_t IPORT_DeltaG;
    uint32_t IPORT_Tsparity;

    // Output port
    uint32_t OPORT_sSRAM; // Connect to state SRAM
    uint32_t OPORT_dSRAM; // Connect to delta-g SRAM
    uint32_t OPORT_NB; // Connect to Neuron block
    uint32_t OPORT_End;
    
    // Internal states
    uint32_t max_idx_;
    uint32_t read_idx_counter_, nb_idx_counter_;
    bool ts_parity_;
    bool is_finish_;

    /* Internal Registers */
    uint64_t deltaG_reg;
    State state_reg;
};
