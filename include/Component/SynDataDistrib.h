#pragma once


#include <TSim/Module/Module.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class SynDataDistrib: public Module
{
public:
    SynDataDistrib (string iname, Component *parent, uint32_t num_propagator);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Input port
    std::vector<uint32_t> IPORT_Syn;
    std::vector<uint32_t> IPORT_Cidx;
    std::vector<uint32_t> IPORT_TS;

    // Output port
    std::vector<uint32_t> OPORT_Syn;
    std::vector<uint32_t> OPORT_TS;

    //
    uint32_t num_prop_;
};
