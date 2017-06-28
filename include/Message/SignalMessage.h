/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct SignalMessage: public Message
{
public:
    // NOTE: must provide default constructor
    SignalMessage () : Message ("SignalMessage") {}

    SignalMessage (uint32_t destrhsid, bool value)
        : Message ("SignalMessage", destrhsid)
    {
        this->value = value;
    }

public:
    bool value;
};
