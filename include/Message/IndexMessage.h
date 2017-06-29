/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct IndexMessage: public Message
{
public:
    // NOTE: must provide default constructor
    IndexMessage () : Message ("IndexMessage") {}

    IndexMessage (uint32_t destrhsid, uint32_t idx)
        : Message ("IndexMessage", destrhsid)
    {
        this->value = idx;
    }

public:
    uint32_t value;
};
