#pragma once

#include <TSim/Pathway/PCIeMessage.h>

#include <cinttypes>
#include <string>

using namespace std;

struct ExamplePCIeMessage: public PCIeMessage
{
private:
    static const uint32_t PAYLOAD_SIZE = 4;     // NOTE: in bytes

public:
    // NOTE: must provide default constructor
    ExamplePCIeMessage () : PCIeMessage () {}

    ExamplePCIeMessage (uint32_t from, uint32_t value, uint32_t devid, uint32_t busid = 0)
        : PCIeMessage ("ExamplePCIeMessage", PAYLOAD_SIZE, devid, busid)
    {
        this->from = from;
        this->value = value;
    }

    uint32_t from;
    uint32_t value;
};
