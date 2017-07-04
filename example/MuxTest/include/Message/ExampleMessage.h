/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


struct ExampleMessage: public Message
{
public:
    // NOTE: must provide default constructor
    ExampleMessage () : Message ("ExampleMessage") {}

    ExampleMessage (uint32_t destrhsid, uint32_t value)
        : Message ("ExampleMessage", destrhsid)
    {
        this->value = value;
    }

public:
    uint32_t value;
};
