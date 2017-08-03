#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct AxonMessage: public Message
{
public:
    // NOTE: must provide default constructor
    AxonMessage () : Message ("AxonMessage") {}

    AxonMessage (uint32_t destrhsid, uint64_t idx, uint16_t len,
            uint16_t delay=0, int16_t tgt=-1, bool is_inh=false)
        : Message ("AxonMessage", destrhsid)
    {
        this->value = idx;
        this->len = len;
        this->delay = delay;
        this->target = tgt;

        this->is_inh = is_inh;
    }

public:
    uint64_t value;
    uint16_t len;
    uint16_t delay;
    int target;

    bool is_inh;
};
