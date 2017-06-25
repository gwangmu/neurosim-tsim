/***
 * TSim Module prototype (header)
 **/

#pragma once

#include <TSim/TSim/Module/Module.h>
#include <TSim/TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


// (TODO) replace 'ExampleModule' to the name you want to put
class ExampleModule: public Module
{
    #if 0
    // (TODO) declare TESTBENCH_VISIBLE to expose module to testbench
    TESTBENCH_VISIBLE;
    #endif

public:
    ExampleModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    #if 0
    // (TODO) create fields to save port IDs
    uint32_t PORT_SPIKEIN;
    uint32_t PORT_SPIKEOUT;

    // (OPTIONAL) insert methods/fields you want to add
    uint32_t counter;
    #endif
};
