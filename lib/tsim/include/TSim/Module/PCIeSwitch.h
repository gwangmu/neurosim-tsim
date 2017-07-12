#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class PCIeSwitch: public Module
{
  public:
    PCIeSwitch (string iname, Component *parent,
            uint32_t n_ports, uint32_t inque_size, uint32_t outque_size,
            uint32_t busid = 0);

    virtual void IsValidConnection (Port *port, Endpoint *endpt);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

    uint64_t GetAccumTXDataSize () { return txsize; }
    uint64_t GetAccumRXDataSize () { return rxsize; }

  private:
    // ports
    uint32_t *PORT_RX;
    uint32_t *PORT_TX;
    uint32_t n_ports;

    // properties
    uint32_t outque_size;
    uint32_t inque_size;
    uint32_t busid;

    // internal state
    uint32_t cur_idx;
    uint32_t next_idx;

    // report state
    uint64_t txsize;
    uint64_t rxsize;
};
