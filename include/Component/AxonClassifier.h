#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class AxonClassifier: public Module
{
public:
    AxonClassifier (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Dram, IPORT_TSparity;

    // Output port
    uint32_t OPORT_Synapse, OPORT_TSparity, OPORT_Axon, OPORT_Sel;
    uint32_t OPORT_BoardID;
    uint32_t OPORT_idle;

    // Internal state
    bool is_idle_;
    bool ts_parity;
};
