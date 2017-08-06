#pragma once

#include <TSim/Pathway/PCIeMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

struct PacketMessage: public PCIeMessage
{
public:
    enum Type { TSEND, AXON };

    PacketMessage () : PCIeMessage ("PacketMessage") {}

    PacketMessage (Type type, uint32_t boardid = -1, uint64_t addr = 0, 
            uint16_t len = 0, uint32_t delay = 0)
        : PCIeMessage ("PacketMessage", 64, boardid)
    {
        this->type = type;
        this->addr = addr;
        this->len = len;
        this->delay = delay;
    }

    virtual PCIeMessage* Clone () { return new PacketMessage (*this); }

    Type type;
    uint64_t addr;
    uint16_t len;
    uint16_t delay;
};
