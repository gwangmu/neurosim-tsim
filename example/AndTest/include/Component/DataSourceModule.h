#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class DataSourceModule: public Module
{
public:
    DataSourceModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr);

private:
    // Port IDs
    uint32_t PORT_DATAOUT;

    // Internal states
    uint32_t counter;
    uint32_t delaycnt;
};
