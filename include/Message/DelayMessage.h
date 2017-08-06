#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

struct DelayMessage: public Message
{
public:
    // NOTE: must provide default constructor
    DelayMessage () : Message ("DelayMessage") {}

    DelayMessage (uint32_t destrhsid, uint64_t idx, uint16_t len=0, 
            uint16_t delay=1)
        : Message ("DelayMessage", destrhsid)
    {
        this->value = idx;
        this->len = len;
        this->delay = delay;
    }

public:
    uint64_t value;
    uint16_t len;
    uint16_t delay;
};
