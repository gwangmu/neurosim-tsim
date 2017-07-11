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
    AxonStorage (string iname, Component *parent);
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
    DramMessage* dram_result;

    // Temp states
    bool is_request;
    uint32_t delay_counter;
    uint32_t delay;
    uint32_t counter; 

    DDR4* dram_spec_;
    Memory<DDR4, Controller> *dram_;

    bool send(uint64_t addr);
    void callback(uint32_t reqID);
};
