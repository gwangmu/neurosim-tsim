#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <Message/DramMessage.h>

#include <Ramulator/Controller.h>
#include <Ramulator/Memory.h>
#include <Ramulator/DDR4.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;

class PseudoStorage: public Module
{
public:
    PseudoStorage (string iname, Component *parent, uint8_t io_buf_size, uint32_t dram_outque_size);
    virtual void Operation (Message **inmsgs, Message **outmsgs, 
            const uint32_t *outque_size, Instruction *instr);

    typedef std::map<int, int> OrgMap;
    static OrgMap test_;

private:
    struct SynMeta
    {
        SynMeta () : 
            tag(-1), base_addr(0), 
            off_board_ofs(0), on_board_ofs(0), entry_cnt(0) {}
        
        SynMeta (uint8_t tag, uint64_t base_addr, uint16_t off_ofs, uint16_t on_ofs) :
            tag(tag), base_addr(base_addr),
            off_board_ofs (off_ofs), on_board_ofs(on_ofs) 
        {
            entry_cnt = on_board_ofs;            
        }

        uint8_t tag; // Message tag.
        uint64_t base_addr;
        uint16_t off_board_ofs; // Routing information
        uint16_t on_board_ofs;  // Synapse information
        uint16_t entry_cnt;
    };
    
    uint32_t PORT_addr, PORT_data;
    uint32_t PORT_idle;

    // Internal State
    uint32_t entry_cnt;
    bool is_idle_;
    bool clk_parity_;

    std::vector<DramMessage*> io_buffer; // For synch I/O clock and DRAM clock
    uint8_t io_counter;

    std::vector<SynMeta> dram_state_; // Store inflight dram request 
    std::vector<uint8_t> reqID_table_; 
    uint8_t req_counter;
    uint8_t reqID_range;
    uint8_t num_streamer_;

    // DRAM specification
    DDR4* dram_spec_;
    Memory<DDR4, Controller> *dram_;
    uint32_t dram_size_;
    uint8_t io_buf_size_;
    uint32_t outque_size_;

    // NeuroSim specification
    uint8_t num_chips_;
    uint8_t num_cores_;

    // Internal function
    bool send(uint8_t reqID, uint64_t addr);
    void callback(uint32_t reqID, uint32_t addr);

    // For debug
    uint16_t idx_counter_;
};
