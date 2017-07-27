#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class Accumulator: public Module
{
  public:
    Accumulator (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

  private:
    /* Port IDs */
    uint32_t PORT_in;
    uint32_t PORT_raddr, PORT_rdata;
    uint32_t PORT_waddr, PORT_wdata;
    uint32_t PORT_TSparity, PORT_idle;

    // Internal state
    bool is_idle_, ts_parity;
    std::list <uint32_t> idx_queue;
};
