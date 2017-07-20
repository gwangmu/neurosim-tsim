#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class CoreAccUnit: public Module
{
  public:
    CoreAccUnit (string iname, Component *parent,
            uint32_t num_propagators);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_coreTS, PORT_accfin;
    std::vector<uint32_t> PORT_syns;
    std::vector<uint32_t> PORT_synTS;

    /* Parameters */
    uint32_t num_propagators_;

    // Internal state
    uint32_t pipeline_depth_;
    uint16_t acc_state_, acc_mask_;
    bool sent_accfin_;
    
    bool coreTS_, synTS_;
    uint8_t syn_rr_;
};
