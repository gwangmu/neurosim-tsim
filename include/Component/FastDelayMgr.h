#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;

class FastDelayMgr: public Module
{
public:
    FastDelayMgr (string iname, Component *parent, uint8_t board_idx);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            Instruction *instr);

private:
    /* Port IDs */
    // Read port  
    uint32_t PORT_input, PORT_TSparity;
    uint32_t PORT_output;
    uint32_t PORT_idle;

    // Parameters
    uint8_t num_delay_;
    uint8_t board_idx_;
    uint64_t base_addr_, board_syns_;
    uint32_t num_neurons_, avg_syns_;

    struct DelayedSpk
    {
        DelayedSpk () {}
        DelayedSpk (uint16_t delay) : delay(delay) {}

        uint16_t delay;
        std::vector<uint32_t> spikes;
    };
    
    std::vector<DelayedSpk> delayed_spks_;
    uint16_t delay_idx_, spk_idx_;
    bool fetch_fin_, is_idle_;

    enum State {IDLE, PROMOTE, RETRIEVE, INSERT, FETCH};
    State state_;
    uint8_t state_counter_;
    bool ts_parity_;

    // Stats
    uint16_t input_n;

    // Hash table
    uint16_t synlen_table[256];
};
