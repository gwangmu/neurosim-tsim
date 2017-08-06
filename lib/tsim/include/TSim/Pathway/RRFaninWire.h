#pragma once

#include <TSim/Pathway/Pathway.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

namespace TSim
{
    class Component;
    class Message;

    class RRFaninWire: public Pathway
    {
    public:
        RRFaninWire (Component *parent, ConnectionAttr conattr, 
                Message *msgproto, uint32_t n_lhs);
        virtual uint32_t NextTargetLHSEndpointID ();
    
    private:
        uint32_t cur_lhsid;
    };
}
