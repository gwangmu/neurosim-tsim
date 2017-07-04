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


class Gate: public Device
{
public:
    Gate (const char *clsname, string iname, Component *parent, 
            Message *msgproto, uint32_t ninput);

    virtual void PreClock (PERMIT(Simulator)) final {};
    virtual void PostClock (PERMIT(Simulator)) final;

    // NOTE: called only if necessary
    virtual Message* Logic (Message const * const *inmsgs) = 0;

private:
    inline void InitInputReadyState () { inready_state = (-1) << ninports; }
    inline void SetInputReadyState (uint32_t id) { inready_state |= (1 << id); }
    inline bool IsInputReady () { return inready_state == -1; }

    uint64_t inready_state;
};
