#pragma once


#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class AxonTransmitter: public Module
{
public:
    AxonTransmitter (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Axon;

    // Output port
    std::vector<uint32_t> OPORT_Axons;
    uint32_t OPORT_idle;

    bool is_idle_;
    uint8_t idle_delay_;
    uint32_t num_propagator;
    uint32_t dram_size;
};
