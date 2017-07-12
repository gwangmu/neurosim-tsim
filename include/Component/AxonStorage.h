#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <Message/DramMessage.h>

#include <Ramulator/Controller.h>
#include <Ramulator/Memory.h>
#include <Ramulator/DDR4.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;

class AxonStorage: public Module
{
public:
    AxonStorage (string iname, Component *parent, uint8_t io_buf_size, uint32_t dram_outque_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

    typedef std::map<int, int> OrgMap;
    static OrgMap test_;

private:
    uint32_t PORT_addr, PORT_data;
    uint32_t PORT_idle;

    // Internal State
    uint32_t entry_cnt;
    bool is_idle_;
    uint32_t req_counter;
    
    std::vector<DramMessage*> io_buffer; // For synch I/O clock and DRAM clock
    uint8_t io_counter;

    // DRAM specification
    DDR4* dram_spec_;
    Memory<DDR4, Controller> *dram_;
    uint32_t dram_size_;
    uint8_t io_buf_size_;
    uint32_t outque_size_;

    // Internal function
    bool send(uint64_t addr);
    void callback(uint32_t reqID, uint32_t addr);
};
