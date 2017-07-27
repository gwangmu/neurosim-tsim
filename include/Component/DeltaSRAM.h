#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <Component/SRAMModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class DeltaStorage: public Module
{
public:
    DeltaStorage (string iname, Component *parent, uint32_t row_size, 
            uint32_t col_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    uint32_t NPORT_raddr, NPORT_rdata;
    uint32_t APORT_raddr, APORT_rdata;
    uint32_t PORT_waddr, PORT_wdata;
    uint32_t PORT_TSparity;

    // Internal State
    bool cur_parity;
    
    /* SRAM features */
    uint32_t row_size_;
    uint32_t col_size_;

    /* Statistics */
    uint32_t read_n, write_n;
};
