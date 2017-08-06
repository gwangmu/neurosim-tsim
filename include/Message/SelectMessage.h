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


struct SelectMessage: public Message
{
public:
    // NOTE: must provide default constructor
    SelectMessage () : Message ("SelectMessage") {}

    SelectMessage (uint32_t destrhsid, uint8_t value)
        : Message ("SelectMessage", destrhsid)
    {
        this->value = value;
    }

public:
    uint8_t value;
};
