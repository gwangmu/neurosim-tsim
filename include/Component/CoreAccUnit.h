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
    CoreAccUnit (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_syn, PORT_accfin;

    /* Parameters */

    // Internal state
    uint32_t pipeline_depth_;
    uint16_t acc_task_;
    bool sent_accfin_;
};
