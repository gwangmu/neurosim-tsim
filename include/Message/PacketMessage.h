#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

enum PacketType {TSEND, AXON};

struct PacketMessage: public Message
{
public:
    // NOTE: must provide default constructor
    PacketMessage () : Message ("PacketMessage") {}

    PacketMessage (uint32_t destrhsid, int32_t rhs,  PacketType type, uint64_t val, int16_t val16=0)
        : Message ("PacketMessage", destrhsid)
    {
        this->rhs = rhs;
        this->value = val;
        this->type = type;
        this->val16 = val16;
    }

public:
    PacketType type;
    int32_t rhs;
    uint64_t value;
    uint16_t val16;
};
