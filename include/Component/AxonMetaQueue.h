#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class AxonMetaQueue: public Module
{
public:
    AxonMetaQueue (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    uint32_t IPORT_NB;
    uint32_t IPORT_Meta;
    uint32_t IPORT_Core_sel;

    // Output port
    uint32_t OPORT_SRAM; 
    uint32_t OPORT_Axon; 
    uint32_t OPORT_Empty;
    
    // Internal states
    bool is_empty;
    int ongoing_jobs;
};
