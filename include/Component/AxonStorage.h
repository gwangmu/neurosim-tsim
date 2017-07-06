#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class AxonStorage: public Module
{
public:
    AxonStorage (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    uint32_t PORT_addr, PORT_data;
    uint32_t PORT_idle;

    // Internal State
    uint32_t entry_cnt;
    bool is_idle_;

    // Temp states
    bool is_request;
    uint32_t delay_counter;
    uint32_t delay;
    uint32_t counter; 

};
