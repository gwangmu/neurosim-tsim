/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct DramReqMessage: public Message
{
public:
    // NOTE: must provide default constructor
    DramReqMessage () : Message ("DramReqMessage") {}

    DramReqMessage (uint32_t destrhsid, uint32_t idx, uint8_t tag=0, 
            uint16_t len=0, bool is_inh=false)
        : Message ("DramReqMessage", destrhsid)
    {
        this->value = idx;
        this->tag = tag;
        this->len = len;
        this->is_inh = is_inh;
    }

public:
    uint32_t value;
    uint8_t tag;
    uint16_t len;

    bool is_inh;
};
