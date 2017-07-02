#pragma once

#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;


class SynDataQueue: public Module
{
public:
    SynDataQueue (string iname, Component *parent, uint32_t max_queue_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    struct SynData
    {
        uint32_t weight;
        uint16_t idx;
    };

    /* Port IDs */
    // Input port
    uint32_t IPORT_Synapse;
    uint32_t IPORT_CoreTS;
    uint32_t IPORT_SynapseTS;

    // Output port
    uint32_t OPORT_Acc; 
    uint32_t OPORT_Empty;
    
    // Internal states
    bool coreTS, synTS;
    bool is_empty;

    uint32_t max_queue_size_;
    std::list <SynData> internal_queue_; // For coreTS != synTS case
};
