#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class FastCoreTSMgr: public Module
{
public:
    FastCoreTSMgr (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    enum Neighbor {DYN, Acc, SDQ};
    // Neuron Block Controller, Neuron Block, Axon Metadata Queue, Accumulator, Synapse Data Queue

    /* Port IDs */
    // Read port  
    uint32_t PORT_DynEnd, PORT_Acc, PORT_SDQ;
    uint32_t PORT_curTS;

    // Write port
    uint32_t PORT_TSparity, PORT_DynFin; 

    /* Internal State */
    bool state[3];
    uint8_t cur_tsparity_, next_tsparity_;
    bool is_dynfin;
};
