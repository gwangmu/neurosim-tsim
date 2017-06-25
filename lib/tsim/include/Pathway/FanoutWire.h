#pragma once

#include <Pathway/Pathway.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Message;


template <uint32_t N_RHS>
class FanoutWire: public Pathway
{
public:
    FanoutWire (Component *parent, ConnectionAttr conattr, Message *msgproto)
        : Pathway ("FanoutWire", parent, conattr, msgproto)
    {
        AddEndpoint ("lhs", Endpoint::LHS, 1);

        if (N_RHS == 0)
            DESIGN_FATAL ("#rhs cannot be zero", GetName().c_str());
        else if (N_RHS == 1)
            DESIGN_WARNING ("'Wire' is recommended for #rhs=1 case", GetName().c_str());

        for (uint32_t i = 0; i < N_RHS; i++)
            AddEndpoint ("rhs" + string(i), Endpoint::RHS, 1);
    }

    virtual uint32_t TargetLHSEndpointID () { return 0; }
};
