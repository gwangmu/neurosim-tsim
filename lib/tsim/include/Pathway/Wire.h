#pragma once

#include <Pathway/Pathway.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Message;


class Wire: public Pathway
{
public:
    Wire (Component *parent, ConnectionAttr conattr, Message *msgproto)
        : Pathway ("Wire", parent, conattr, msgproto)
    {
        AddEndpoint ("lhs", Endpoint::LHS, 1);
        AddEndpoint ("rhs", Endpoint::RHS, 1);
    }

    virtual uint32_t TargetLHSEndpointID () { return 0; }
};
