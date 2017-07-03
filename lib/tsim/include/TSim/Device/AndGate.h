#pragma once

#include <TSim/Device/Gate.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Message;
class Component;
class Simulator;


class AndGate: public Gate
{
public:
    AndGate (string iname, Component *parent, uint32_t ninput)
        : Gate ("Gate", iname, parent, Prototype<IntegerMessage>::Get()) {}

    virtual Message* Logic (Message const * const *inmsgs);

private:
    IntegerMessage cached_output;
};
