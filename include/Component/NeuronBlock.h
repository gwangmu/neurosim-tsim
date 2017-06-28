#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class NeuronBlock: public Module
{
  public:
    NeuronBlock (string iname, Component *parent, uint32_t depth);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    // Input ports
    uint32_t IPORT_Nidx;
    uint32_t IPORT_State;
    uint32_t IPORT_DeltaG;

    // Output ports
    uint32_t OPORT_Nidx;
    uint32_t OPORT_Spike;

    /* Pipeline */
    uint32_t pipeline_depth_;
    uint32_t pipeline_state_;

    /* Spike traces */
    ifstream *spike_trace_file;
    vector<vector<int>> spike_traces_;
    
    vector<int> spike_trace_;

    // Internal state
    uint32_t mask_;
};
