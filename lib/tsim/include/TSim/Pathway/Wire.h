#pragma once

#include <TSim/Pathway/Pathway.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Component;
class Message;


class Wire: public Pathway
{
public:
    Wire (Component *parent, ConnectionAttr conattr, Message *msgproto);
    virtual uint32_t NextTargetLHSEndpointID ();
};
