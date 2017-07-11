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
    uint32_t OPORT_Axon, OPORT_idle;

    bool is_idle_;
};
