#include <Component/AxonStorage.h>

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

USING_TESTBENCH;

AxonStorage::AxonStorage (string iname, Component* parent, uint8_t io_buf_size, uint32_t dram_outque_size)
    : Module ("AxonStorage", iname, parent, 0)
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
   
    this->outque_size_ = dram_outque_size;
    this->io_buf_size_ = io_buf_size;
    io_buffer.resize(io_buf_size_);

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

    INFO_PRINT ("[DRAM] Initialize %s DRAM", standard.c_str());
    Register::Attr regattr (64, dram_size_);
    SetRegister (new DramFileRegister (Register::SRAM, regattr));
}


void AxonStorage::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_addr]);

    if(clk_parity_)
    {
        if(raddr_msg)
        {
            uint32_t read_addr = raddr_msg->value;

            if(((entry_cnt + io_buf_size_ + GetOutQueSize(PORT_data) + 1) > 
                        outque_size_) || (!send(read_addr)))
            {
                INFO_PRINT("[DRAM] DRAM is full (entry %d/ outque %u)", entry_cnt, /**outque_size*/ GetOutQueSize(PORT_data)); 
                inmsgs[PORT_addr] = nullptr;
            }
            else
            {
                INFO_PRINT("[DRAM] Receive read request %d", entry_cnt);
                outmsgs[PORT_data] = Message::RESERVE();
                entry_cnt += io_buf_size_;
            }
        }

        if(!is_idle_ && (entry_cnt==0) && 
                /**outque_size*/ (GetOutQueSize(PORT_data) == 0) && io_counter == 0)
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
            INFO_PRINT ("[DRAM] outque %u(%u)/%u", /**outque_size*/ GetOutQueSize(PORT_data), entry_cnt, outque_size_);

            io_buffer[idx] = nullptr;
        }
        io_counter--;
        entry_cnt--;
    }

    clk_parity_ = !clk_parity_;
}


bool AxonStorage::send (uint64_t addr)
{
    ramulator::Request::Type req_type = ramulator::Request::Type::READ;

    uint32_t reqID = req_counter++;
    auto dram_complete = [reqID, addr, this] (ramulator::Request& r) {this->callback(reqID, addr);};

    ramulator::Request req = ramulator::Request(addr, req_type, dram_complete);

    if(dram_->send(req))
    {
        INFO_PRINT ("[DRAM] Send dram request (ID: %u, addr: %lu)", reqID, addr);
        return true;
    }
    else
        return false;
}

void AxonStorage::callback (uint32_t reqID, uint32_t addr)
{
    // Read data
    for (int i=0; i<io_buf_size_; i++)
    {
        const DramRegisterWord *word =
            static_cast<const DramRegisterWord *>(GetRegister()->GetWord (addr + i));
        uint64_t data = word->value;
      
        bool valid = (data >> 63) & 1;
        if(!valid) continue;

        bool intra_board;
        uint64_t val32;
        uint32_t destrhsid;
        uint8_t target_idx; 
        uint16_t val16;
        
        // Synapse type
        bool syn_type = (data >> 62) & 1;
        if(!syn_type)
        {
            intra_board = false;
            val32 = (data >> 21) & (0xffffffff);
            destrhsid = (data >> 18) & (0x7);
            target_idx  = (data >> 15) & (0x7);
            val16 = (data & (0x7fff));

            INFO_PRINT ("[DRAM] destination %u/%u/%u", destrhsid, target_idx, val16);
        }
        else
        {
            INFO_PRINT ("[DRAM] Data: %lu, %lu", data, data>>63);
            SIM_ERROR ("Not Implemented. (Routing information)", GetFullName().c_str());
        }

       
        //outmsgs[PORT_data] = new DramMessage (destrhsid, val32, val16,
        //        intra_board, target_idx);
        if(io_buffer[i] == nullptr)
        {
            DramMessage::Type type;
            if(intra_board)
                type = DramMessage::REMOTE;
            else
                type = DramMessage::SYNAPSE;
            
            io_buffer[i] = new DramMessage (destrhsid, val32, val16,
                    type, target_idx);
        }
        else
        {
            SIM_ERROR ("Dram output is overwritten", GetFullName().c_str());
        }
    }
    io_counter = io_buf_size_;

    PRINT ("[DRAM] Response dram request (reqID: %u, addr: %u)", reqID, addr);
}




