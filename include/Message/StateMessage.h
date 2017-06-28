/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct State
{
    uint32_t value;
};


struct StateMessage: public Message
{
public:
    // NOTE: must provide default constructor
    StateMessage () : Message ("StateMessage") {}

    StateMessage (uint32_t destrhsid, State value)
        : Message ("StateMessage", destrhsid)
    {
        this->value = value;
    }

public:
    State value;
};
