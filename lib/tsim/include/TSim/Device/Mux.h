#pragma once

#include <TSim/Device/Device.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Message;
class Component;
class Simulator;


class Mux: public Device
{
public:
    Mux (string iname, Component *parent, Message *msgproto, uint32_t ninput);
    virtual void PreClock (PERMIT(Simulator)) final;
    virtual void PostClock (PERMIT(Simulator)) final;
};
