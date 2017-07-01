#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <Component/SRAMModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class DeltaSRAM: public SRAMModule
{
public:
    DeltaSRAM (string iname, Component *parent, uint32_t row_size, 
            uint32_t col_size, bool parity);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    uint32_t PORT_TSparity;

    // Internal State
    bool cur_parity;
    bool SRAM_parity;


};

class DeltaStorage: public Component
{
public:
    DeltaStorage (string iname, Component *parent, uint32_t row_size, uint32_t col_size);
};
