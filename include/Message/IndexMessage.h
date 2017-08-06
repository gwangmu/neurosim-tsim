/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


struct IndexMessage: public Message
{
public:
    // NOTE: must provide default constructor
    IndexMessage () : Message ("IndexMessage") {}

    IndexMessage (uint32_t destrhsid, uint32_t idx, uint8_t tag=0)
        : Message ("IndexMessage", destrhsid)
    {
        this->value = idx;
        this->tag = tag;
    }

public:
    uint32_t value;
    uint8_t tag;
};
