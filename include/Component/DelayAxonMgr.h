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
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr);

private:
    /* Port IDs */
    // Read port  
    uint32_t PORT_input, PORT_TSparity;
    uint32_t PORT_output;
    uint32_t PORT_idle;

    // Storage port
    uint32_t PORT_raddr, PORT_rdata;
    uint32_t PORT_waddr, PORT_wdata;

    /* Internal state */
    enum State {IDLE, PROMOTE, RETRIEVE, INSERT, FETCH};
    State state_, next_state_;
    uint8_t state_counter_;
        
    bool ts_parity_;
    bool fetch_fin_, is_idle_;

    uint32_t readFLA_, readPLA_, readDALA_;

    // RETRIEVE / INSERT state
    struct PLE // Pointer List Element
    {
        PLE () : addr(0), next_ptr(0), delay(0), list_head(0) {}
        PLE (uint32_t addr, uint32_t next_ptr, uint16_t delay,
                uint32_t list_head) 
            : addr(addr), next_ptr (next_ptr), abs_delay(delay),
              list_head (list_head) {}
                    

        uint32_t addr;
        uint32_t next_ptr;
        uint16_t abs_delay; // Absolute delay
        uint32_t list_head;
        
        uint16_t delay; // relative delay
    };
    uint16_t in_delay_, in_len_;
    uint64_t in_axaddr_;
    
    uint16_t delay_buf_;
    uint16_t addr_ptr_, prehead_ptr_;
    PLE prePLE_, postPLE_; // for insert head in list

    // Internal register
    std::vector<uint32_t> internal_reg_;
    uint16_t reg_head_;
    uint32_t regFLA_, regPLA_;
    uint16_t reg_size_;

    // State function
    int16_t Retrieve (Message **inmsgs, Message **outmsgs, int state);
    int16_t Insert (Message **inmsgs, Message **outmsgs, int state);
    int16_t Promote (Message **inmsgs, Message **outmsgs, int state);
    int16_t Fetch (Message **inmsgs, Message **outmsgs, int state);

    // Statistics
    int entry_cnt_;
};
