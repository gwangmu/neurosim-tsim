#pragma once

#include <TSim/Pathway/Pathway.h>
#include <TSim/Base/IssueCount.h>
#include <TSim/Simulation/Simulator.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

namespace TSim
{
    class Component;
    class Message;

    class Link: public Pathway
    {
    public:
        Link (Component *parent, ConnectionAttr conattr, double link_speed,
                Message *msgproto);
    
        void ApplyReferenceClockPeriod (uint32_t period);
        virtual uint32_t GetBitWidth ();
        virtual uint32_t NextTargetLHSEndpointID ();
    
    private:
        uint32_t bitwidth;
        double link_speed;      // NOTE: in GT/s
        double factor;
    };
}
