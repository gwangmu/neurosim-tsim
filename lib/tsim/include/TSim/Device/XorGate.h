#pragma once

#include <TSim/Device/Gate.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

namespace TSim
{
    class Message;
    class Component;
    class Simulator;

    class XorGate: public Gate
    {
    public:
        XorGate (string iname, Component *parent, uint32_t ninput)
            : Gate ("XorGate", iname, parent, Prototype<IntegerMessage>::Get(), ninput), 
            cached (false) {}
    
        virtual Message* Logic (Message const * const *inmsgs);
    
    private:
        IntegerMessage cached_output;
        bool cached;
    };
}
