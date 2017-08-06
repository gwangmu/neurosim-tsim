/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

typedef uint64_t State;

// struct State
// {
//     uint32_t value;
// };


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
