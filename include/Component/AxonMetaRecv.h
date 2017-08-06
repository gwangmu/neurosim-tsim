#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class AxonMetaRecv: public Module
{
public:
    AxonMetaRecv (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_Axon, IPORT_Bypass;

    // Output port
    uint32_t OPORT_Axon, OPORT_idle, OPORT_delay;

    bool is_idle_;
};
