#pragma once


#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class PacketDecoder: public Module
{
public:
    PacketDecoder (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Packet;

    // Output port
    uint32_t OPORT_TSEnd;
    std::vector<uint32_t> OPORT_Axons;
    std::vector<uint32_t> OPORT_Bypass;

    // Parameters
    uint8_t num_propagators_;
};
