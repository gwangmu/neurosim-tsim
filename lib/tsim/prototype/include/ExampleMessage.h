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

    #if 0
    // (TODO) create constructors
    ExampleMessage (uint32_t destrhsid) 
        : Message ("ExampleMessage", destrhsid) 
    {
        value = 0; 
    }

    ExampleMessage (uint32_t destrhsid, uint32_t value)
        : Message ("ExampleMessage", destrhsid)
    {
        this->value = value;
    }
    #endif

public:
    #if 0
    // (TODO) insert fields
    uint32_t value;
    #endif
}
