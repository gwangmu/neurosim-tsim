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

    AxonMessage (uint32_t destrhsid, uint64_t idx, uint16_t len=0)
        : Message ("AxonMessage", destrhsid)
    {
        this->value = idx;
        this->len = len;
    }

public:
    uint64_t value;
    uint16_t len;
};