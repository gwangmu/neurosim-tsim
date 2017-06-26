#pragma once

#include <TSim/Pathway/Pathway.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Message;


class FanoutWire: public Pathway
{
public:
    FanoutWire (Component *parent, ConnectionAttr conattr, 
            Message *msgproto, uint32_t n_rhs);
    virtual uint32_t NextTargetLHSEndpointID ();
};
