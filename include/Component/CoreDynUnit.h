#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class CoreDynUnit: public Module
{
  public:
    CoreDynUnit (string iname, Component *parent, 
            uint32_t num_neurons, uint32_t depth, uint16_t core_idx);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_coreTS, PORT_dynfin;
    uint32_t PORT_axon;

    /* Spike traces */
    std::list<int> spike_trace_;

    /* Parameters */
    uint32_t num_neurons_;
    uint32_t row_size_, col_size_;
    uint32_t avg_synapses_;
    uint16_t min_delay_;
    
    uint16_t core_idx_, prop_idx_;
    uint64_t base_addr_;

    // Internal state
    uint32_t pipeline_depth_;
    uint16_t spike_state_, spike_mask_;
    uint16_t pipe_state_, pipe_mask_;
    bool is_idle_;

    bool ts_parity_;
    uint32_t idx_counter_;

    // Pseudo-random table
    uint16_t synlen_table[1024];
};
