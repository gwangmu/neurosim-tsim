#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct DelayMetaMessage: public Message
{
public:
    // NOTE: must provide default constructor
    DelayMetaMessage () : Message ("DelayMetaMessage") {}

    DelayMetaMessage (uint32_t destrhsid, uint32_t addr1, uint64_t addr2, 
            uint16_t val16)
        : Message ("DelayMetaMessage", destrhsid)
    {
        this->next_addr = addr1;
        this->addr_sub = addr2;
        this->val16 = val16;
    }

public:
    uint64_t next_addr;
    uint16_t addr_sub; // AxAddr (DALE), List head (PLE)
    uint16_t val16;
};
