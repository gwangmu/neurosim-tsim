#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace TSim;

class NeuronBlock: public Module
{
  public:
    NeuronBlock (string iname, Component *parent, uint32_t depth);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_in, PORT_out;
    uint32_t OPORT_waddr, OPORT_wdata, OPORT_idle;

    /* Spike traces */
    std::list<int> spike_trace_;

    // Internal state
    uint32_t pipeline_depth_;
    uint32_t idle_time_;
    bool is_idle_;
};
