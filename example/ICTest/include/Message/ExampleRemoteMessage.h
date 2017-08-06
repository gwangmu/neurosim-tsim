#pragma once

#include <TSim/Pathway/RemoteMessage.h>

#include <cinttypes>
#include <string>

using namespace std;
using namespace TSim;

struct ExampleRemoteMessage: public RemoteMessage
{
private:
    static const uint32_t PAYLOAD_SIZE = 32;     // NOTE: in bytes

public:
    // NOTE: must provide default constructor
    ExampleRemoteMessage () : RemoteMessage ("ExampleRemoteMessage") {}

    ExampleRemoteMessage (uint32_t from, uint32_t value, uint32_t devid, uint32_t busid = 0)
        : RemoteMessage ("ExampleRemoteMessage", PAYLOAD_SIZE, devid, busid)
    {
        this->from = from;
        this->value = value;
    }

    virtual RemoteMessage* Clone () { return new ExampleRemoteMessage(*this); }

    uint32_t from;
    uint32_t value;
};
