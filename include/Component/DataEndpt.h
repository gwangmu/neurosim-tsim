#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


template <class M>
class DataEndptModule: public Module
{
public:
    DataEndptModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr) {}

private:
    // Port IDs
    uint32_t PORT_DATAOUT;
};
