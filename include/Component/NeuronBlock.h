#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class NeuroSimTestbench;

class NeuronBlock: public Module
{
    VISIBLE_TO(NeuroSimTestbench);

  public:
    NeuronBlock (string iname, Component *parent, uint32_t depth);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    // Input ports
    uint32_t PORT_in, PORT_out;

    uint32_t IPORT_Nidx;
    uint32_t IPORT_State;
    uint32_t IPORT_DeltaG;

    // Output ports
    uint32_t OPORT_Nidx;
    uint32_t OPORT_Spike;

    /* Spike traces */
    std::list<int> spike_trace_;

    // Internal state
    std::list<uint32_t> pipelined_idx_;

    uint32_t pipeline_depth_;
};
