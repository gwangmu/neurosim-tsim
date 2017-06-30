#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class SRAMModule: public Module
{
public:
    SRAMModule (const char* clsname, string iname, Component *parent, uint32_t row_size, uint32_t col_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr) = 0;

protected:
    /* Port IDs */
    // Read port  
    uint32_t RPORT_addr;
    uint32_t RPORT_data;

    // Write port
    uint32_t WPORT_addr; 
    uint32_t WPORT_data; 

    /* SRAM features */
    uint32_t row_size_;
    uint32_t col_size_;

    /* Statistics */
    uint32_t read_n, write_n;
};
