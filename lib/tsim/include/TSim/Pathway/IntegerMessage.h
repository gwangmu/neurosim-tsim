#pragma once

#include <TSim/Pathway/Message.h>
#include <cinttypes>

using namespace std;

struct IntegerMessage : public Message
{
public:
    // NOTE: fixed to broadcast
    IntegerMessage () 
        : Message ("IntegerMessage", TOGGLE, -1), value (-1) {}
    IntegerMessage (uint64_t value)
        : Message ("IntegerMessage", TOGGLE, -1), value (value) {}

    uint64_t value;
};
