#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct PCIeMessage;

class PCIeSwitch: public Module
{
  public:
    PCIeSwitch (string iname, Component *parent, PCIeMessage *msgproto,
            uint32_t n_ports, uint32_t inque_size, uint32_t outque_size,
            uint32_t busid = 0);

    virtual bool IsValidConnection (Port *port, Endpoint *endpt);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

    uint64_t GetAccumTrafficBytes () { return traffic_size_bits / 8; }

  private:
    // ports
    uint32_t *PORT_RX;
    uint32_t *PORT_TX;
    uint32_t n_ports;
    uint64_t rxvalid;

    // properties
    uint32_t outque_size;
    uint32_t inque_size;
    uint32_t busid;

    // internal state
    uint32_t cur_idx;
    uint32_t next_idx;

    // report state
    uint64_t traffic_size_bits;
};
