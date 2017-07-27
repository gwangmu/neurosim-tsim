#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class FastSynQueue: public Module
{
  public:
    FastSynQueue (string iname, Component *parent, 
            uint32_t num_propagators);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_coreTS, PORT_empty;
    std::vector<uint32_t> PORT_syns;
    std::vector<uint32_t> PORT_synTS;
    std::vector<uint32_t> PORT_acc;

    /* Parameters */
    uint32_t num_propagators_;

    // Internal state
    std::vector<uint16_t> syn_queue_size_;
    
    uint32_t pipeline_depth_;
    uint16_t acc_task_;
   
    std::vector<bool> queue_state_;
    bool is_empty_, sent_accfin_;

    bool coreTS_;
    std::vector<bool> synTS_;
};
