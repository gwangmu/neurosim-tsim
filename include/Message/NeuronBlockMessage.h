/***
 * TSim Message prototype (header)
 **/

#pragma once

#include <TSim/Pathway/Message.h>
#include <Message/StateMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


struct NeuronBlockInMessage: public Message
{
public:
    // NOTE: must provide default constructor
    NeuronBlockInMessage () : Message ("NeuronBlockInMessage") {}

    NeuronBlockInMessage (uint32_t destrhsid, uint32_t idx, State state, uint64_t deltaG)
        : Message ("NeuronBlockIMessage", destrhsid)
    {
        this->idx = idx;
        this->state = state;
        this->deltaG = deltaG;
    }

public:
    uint32_t idx;
    State state;
    uint64_t deltaG;
};

struct NeuronBlockOutMessage: public Message
{
public:
    // NOTE: must provide default constructor
    NeuronBlockOutMessage () : Message ("NeuronBlockOutMessage") {}

    NeuronBlockOutMessage (uint32_t destrhsid, uint32_t idx, bool spike)
        : Message ("NeuronBlockOutMessage", destrhsid)
    {
        this->value = idx;
        this->spike = spike;
    }

public:
    uint32_t value;
    bool spike;
};
