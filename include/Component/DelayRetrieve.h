#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class DelayRetrieve: public Module
{
public:
    DelayRetrieve (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // 
    uint32_t PORT_delay;
    
    // Storage port
    uint32_t PORT_raddr, PORT_rdata;

    /* Internal state */
    uint32_t readFLA_, readPLA_;

    // RETRIEVE / INSERT state
    uint16_t in_delay_, in_len_, in_axaddr_;
};
