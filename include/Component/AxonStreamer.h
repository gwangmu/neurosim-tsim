#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class AxonStreamer: public Module
{
public:
    AxonStreamer (string iname, Component *parent, uint8_t io_buf_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Axon;

    // Output port
    uint32_t OPORT_Addr, OPORT_idle; 

    // DRAM parameters
    uint32_t read_bytes;

    // Internal state
    bool is_idle_;
    uint32_t base_addr_;
    uint16_t ax_len_;
    uint32_t read_addr_;
};
