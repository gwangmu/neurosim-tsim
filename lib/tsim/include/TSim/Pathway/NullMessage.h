#pragma once

#include <TSim/Pathway/Message.h>
#include <cinttypes>

using namespace std;

struct NullMessage : public Message
{
public:
    NullMessage () 
        : Message ("NullMessage", TOGGLE, -1) {}
};
