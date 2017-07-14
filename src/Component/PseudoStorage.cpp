#include <Component/PseudoStorage.h>

#include <Register/DramFileRegister.h>
#include <Register/DramRegisterWord.h>
#include <Message/IndexMessage.h>
#include <Message/DramMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Ramulator/DRAM.h>
#include <Ramulator/Processor.h>
#include <Ramulator/SpeedyController.h>

#include <cstdlib>

USING_TESTBENCH;

PseudoStorage::PseudoStorage (string iname, Component* parent, uint8_t io_buf_size, uint32_t dram_outque_size)
    : Module ("PseudoStorage", iname, parent, 0)
{
    SetClock ("ddr");

    PORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    PORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<DramMessage>::Get());
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    entry_cnt = 0;
    is_idle_ = false;
    clk_parity_ = true;

    req_counter = 0;

    /* Initialize parameters */
    dram_size_ = GET_PARAMETER (dram_size);
    num_chips_ = GET_PARAMETER (num_chips);
    num_cores_ = GET_PARAMETER (num_cores);

    this->outque_size_ = dram_outque_size;
    this->io_buf_size_ = io_buf_size;
    io_buffer.resize(io_buf_size_);

    reqID_range = 20;
    dram_state_.resize(reqID_range);
    
    reqID_table_.clear();
    num_streamer_ = 4;
    for (int i=0; i<num_streamer_; i++) // 4: # of streamer
        reqID_table_.push_back(0);

    srand (1015);

    /* Initialize DRAM (ramulator */
    string config_path;
    config_path = "lib/ramulator/configs/DDR4-config.cfg";

    ramulator::Config configs(config_path);

    const std::string& standard = configs["standard"];

    configs.add("trace_type", "DRAM");
    configs.set_core_num(1);

    /** Ramulator Spec **/ 
    ramulator::DDR4::Org org = ramulator::DDR4::Org::DDR4_4Gb_x8;
    ramulator::DDR4::Speed speed = ramulator::DDR4::Speed::DDR4_2400R;

    dram_spec_ = new DDR4 (org, speed);
    
    int C = configs.get_channels(), R = configs.get_ranks();

    dram_spec_->set_channel_number(C);
    dram_spec_->set_rank_number(R);
        
    
    std::vector<Controller<DDR4>*>ctrls;
    for(int c=0; c < C; c++)
    {
      DRAM<DDR4> *channel = new DRAM<DDR4>(dram_spec_, DDR4::Level::Channel);
      channel->id = c;
      channel->regStats("");
      Controller<DDR4>* ctrl = new Controller<DDR4>(configs, channel);
      ctrls.push_back(ctrl);
    }

    dram_ = new Memory<DDR4, Controller> (configs, ctrls);

    // Register
    INFO_PRINT ("[DRAM] Initialize %s DRAM", standard.c_str());
    Register::Attr regattr (64, dram_size_);
    SetRegister (new DramFileRegister (Register::SRAM, regattr));
    
}


void PseudoStorage::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_addr]);

    if(clk_parity_)
    {
        if(raddr_msg)
        {
            // Output queue is full
            if(((entry_cnt + io_buf_size_) > outque_size_))
                inmsgs[PORT_addr] = nullptr;
            
            uint32_t read_addr = raddr_msg->value;
            
            // Check tag
            uint8_t tag = raddr_msg->tag;
            uint8_t reqID = reqID_table_[tag%num_streamer_];

            // Get metadata
            SynMeta synmeta = dram_state_[reqID];

            // New streaming is coming 
            if(synmeta.tag != tag) 
            {
                // Update reqID table
                reqID = req_counter;
                reqID_table_[tag%num_streamer_] =reqID;
                
                req_counter = (req_counter +1) % reqID_range;

                // Update dram state
                if (unlikely(dram_state_[reqID].entry_cnt != 0))
                {
                    SIM_ERROR ("Dram state is overwritten", GetFullName().c_str());
                    return;
                }
               
                INFO_PRINT ("[DRAM] Update metadata (addr: %x, reqID: %d, tag: %d)", 
                        read_addr, (int)reqID, tag);

                const DramRegisterWord *word =
                    static_cast<const DramRegisterWord *>(GetRegister()->GetWord (read_addr));
                uint64_t data = word->value;

                uint16_t off_ofs = (data >> 16) & (0xffff);
                uint16_t on_ofs = (data) & (0xffff);

                synmeta = SynMeta (tag, read_addr, off_ofs, on_ofs);
                dram_state_[reqID] = synmeta;      
                
            }
            else
            {
                INFO_PRINT ("[DRAM] Receive request (addr %x, reqID: %d, tag: %d",
                        read_addr, (int)reqID, tag);
            }

            if(!send(reqID, read_addr))
                inmsgs[PORT_addr] = nullptr;
            else
            {
                INFO_PRINT("[DRAM] Receive read request %d", entry_cnt);
                outmsgs[PORT_data] = Message::RESERVE();
                entry_cnt += io_buf_size_;
            }
        }

        if(!is_idle_ && (entry_cnt==0) && 
                (*outque_size==0) && io_counter == 0)
        {
            INFO_PRINT ("[DRAM] DRAM is idle");
            is_idle_ = true;
            outmsgs[PORT_idle] = new IntegerMessage (1);
        }
        else if(is_idle_ && (entry_cnt != 0))
        {
            INFO_PRINT ("[DRAM] DRAM is busy");
            is_idle_ = false;
            outmsgs[PORT_idle] = new IntegerMessage (0);
        }
        
        dram_->tick();
    }
    else
    {
        raddr_msg = nullptr;
    }

    if(io_counter > 0)
    {
        int idx = (io_buf_size_ - io_counter);
        
        if(io_buffer[idx] != nullptr)
        {
            outmsgs[PORT_data] = io_buffer[idx];
            
            INFO_PRINT ("[DRAM] Send dram data to %u/%u/%u", 
                    io_buffer[idx]->dest_idx, 
                    io_buffer[idx]->target_idx, 
                    io_buffer[idx]->val16);
            INFO_PRINT ("[DRAM] outque %u/%u", *outque_size, outque_size_);

            io_buffer[idx] = nullptr;
        }
        io_counter--;
        entry_cnt--;
    }

    clk_parity_ = !clk_parity_;
}


bool PseudoStorage::send (uint8_t reqID, uint64_t addr)
{
    ramulator::Request::Type req_type = ramulator::Request::Type::READ;

    auto dram_complete = [reqID, addr, this] (ramulator::Request& r) {this->callback(reqID, addr);};

    addr *= 8;
    ramulator::Request req = ramulator::Request(addr, req_type, dram_complete);

    if(dram_->send(req))
    {
        INFO_PRINT ("[DRAM] Response to request (ID: %u, addr: %x)", reqID, addr);
        return true;
    }
    else
        return false;
}

void PseudoStorage::callback (uint32_t reqID, uint32_t addr)
{
    SynMeta synmeta = dram_state_[reqID]; 
    if (unlikely(synmeta.entry_cnt == 0))
    {
        SIM_ERROR ("Dram state is invalid", GetFullName().c_str());
        return;
    }
    uint16_t offset = addr - synmeta.base_addr;
    
    // Read data
    for (int i=0; i<io_buf_size_; i++)
    {
        bool intra_board;
        uint64_t val32;
        uint32_t destrhsid;
        uint8_t target_idx; 
        uint16_t val16;
        
        if(offset + i < synmeta.off_board_ofs)
        {
            INFO_PRINT ("[DRAM] reqID: %d, addr: %u", reqID, addr);
            SIM_ERROR ("Not Implemented. (Routing information)", GetFullName().c_str());
            dram_state_[reqID].entry_cnt--;
            return;
        }
        else if (offset + i >= synmeta.on_board_ofs)
        {
            INFO_PRINT ("[DRAM] Empty data");
            continue;
        }
        else
        {
            intra_board = false;
            val32 = 20170713;
            val16 = 1529;

            destrhsid = rand() % num_chips_;
            target_idx = rand() % num_cores_;
            
            INFO_PRINT ("[DRAM] destination %u/%u/%u", destrhsid, target_idx, val16);
            
            dram_state_[reqID].entry_cnt--;
        }

        if(io_buffer[i] == nullptr)
        {
            io_buffer[i] = new DramMessage (destrhsid, val32, val16,
                    intra_board, target_idx);
        }
        else
        {
            SIM_ERROR ("Dram output is overwritten", GetFullName().c_str());
        }
    }
    io_counter = io_buf_size_;

    INFO_PRINT ("[DRAM] Receive dram request (reqID: %u, addr: %u)", reqID, addr);
}




