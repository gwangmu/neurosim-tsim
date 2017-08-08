#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace TSim;

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
    uint8_t num_delay_, min_delay_;
    uint8_t board_idx_;
    uint64_t base_addr_, board_syns_;
    uint32_t num_neurons_, neurons_per_board_, neurons_per_prop_, avg_syns_;
    uint32_t inh_neurons_;

    struct DelayedSpk
    {
        DelayedSpk () {}
        DelayedSpk (uint16_t delay) : delay(delay) {}

        uint16_t delay;
        bool is_inh;
        std::vector<uint32_t> spikes;
    };
    
    std::list<DelayedSpk> delayed_spks_;
    std::list<DelayedSpk>::iterator delay_it_;
    uint16_t spk_idx_;
    bool fetch_fin_, is_idle_, is_start_;

    enum State {IDLE, PROMOTE, RETRIEVE, INSERT, FETCH, START};
    State state_;
    uint8_t state_counter_;
    bool ts_parity_;

    // Stats
    uint16_t input_n;
    uint32_t data_cnt;

    // Hash table
    uint16_t synlen_table[256];
};
