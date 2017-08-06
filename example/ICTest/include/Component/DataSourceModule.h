#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

// FOR TEST PURPOSE
#define MAX_TEST_SOURCES 16

class DataSourceModule: public Module
{
public:
    DataSourceModule (string iname, Component *parent, string clock, uint32_t testid);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    // Port IDs
    uint32_t PORT_DATAIN;
    uint32_t PORT_DATAOUT;

    // Internal states
    uint32_t counter;
    uint32_t testid;
};
