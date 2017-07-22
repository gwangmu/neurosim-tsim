#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class DelayAxonMgr: public Module
{
public:
    DelayAxonMgr (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

private:
    /* Port IDs */
    // Read port  
    uint32_t PORT_input, PORT_TSparity;
    uint32_t PORT_output;

    // Storage port
    uint32_t PORT_raddr, PORT_rdata;
    uint32_t PORT_waddr, PORT_wdata;

    /* Internal state */
    enum State {IDLE, PROMOTE, RETRIEVE, INSERT, FETCH};
    State state_, next_state_;
    uint8_t state_counter_;
        
    bool ts_parity_;
    bool fetch_fin_;

    uint32_t readFLA_, readPLA_, readDALA_;

    // RETRIEVE / INSERT state
    struct PLE // Pointer List Element
    {
        uint32_t addr;
        uint32_t next_ptr;
        uint16_t delay;
        uint32_t list_head;
    };
    uint16_t in_delay_, in_len_, in_axaddr_;
    
    uint16_t delay_buf_;
    uint16_t addr_ptr_, prehead_ptr_;
    PLE prePLE_, postPLE_; // for insert head in list

    // Internal register
    std::vector<uint32_t> internal_reg_;
    uint16_t reg_head_;
    uint32_t regFLA_, regPLA_;
    uint16_t reg_size_;
};
