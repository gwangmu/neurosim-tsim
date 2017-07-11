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
#include <Ramulator/Statistics.h>
#include <Ramulator/SpeedyController.h>

USING_TESTBENCH;

AxonStorage::AxonStorage (string iname, Component* parent)
    : Module ("AxonStorage", iname, parent, 0)
{

    PORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    PORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<DramMessage>::Get());
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    entry_cnt = 0;
    is_idle_ = false;

    this->counter = 0;

    req_counter = 0;

    /* Initialize parameters */
    dram_size_ = GET_PARAMETER (dram_size);


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

    DEBUG_PRINT("Make DDR4 instance (org %d, speed %d)",
            int(org), int(speed));
    dram_spec_ = new DDR4 (org, speed);
    
    int C = configs.get_channels(), R = configs.get_ranks();

    dram_spec_->set_channel_number(C);
    dram_spec_->set_rank_number(R);
    
    DEBUG_PRINT("Make DDR4 controller");
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

    DEBUG_PRINT ("[DRAM] Initialize %s DRAM", standard.c_str());

    /* DRAM data */
    Register::Attr regattr (64, dram_size_);
    SetRegister (new DramFileRegister (Register::SRAM, regattr));
}


void AxonStorage::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;
        entry_cnt++;

        DEBUG_PRINT("[DRAM] Receive read request");

        if(!send (read_addr))
        {
            inmsgs[PORT_addr] = nullptr;
        }
        else
            outmsgs[PORT_data] = Message::RESERVE();
    }

    if(!is_idle_ && (entry_cnt==0) && (*outque_size==0))
    {
        DEBUG_PRINT ("[DRAM] DRAM is idle");
        is_idle_ = true;
        outmsgs[PORT_idle] = new IntegerMessage (1);
    }
    else if(is_idle_ && (entry_cnt != 0))
    {
        DEBUG_PRINT ("[DRAM] DRAM is busy");
        is_idle_ = false;
        outmsgs[PORT_idle] = new IntegerMessage (0);
    }
    
    dram_->tick();
    if(dram_result)
    {
        outmsgs[PORT_data] = dram_result;
        dram_result = nullptr;
    }
}


bool AxonStorage::send (uint64_t addr)
{
    ramulator::Request::Type req_type = ramulator::Request::Type::READ;

    uint32_t reqID = req_counter++;
    auto dram_complete = [reqID, addr, this] (ramulator::Request& r) {this->callback(reqID, addr);};

    ramulator::Request req = ramulator::Request(addr, req_type, dram_complete);

    if(dram_->send(req))
    {
        DEBUG_PRINT ("[DRAM] Send dram request (ID: %u, addr: %lu)", reqID, addr);
        return true;
    }
    else
        return false;
}

void AxonStorage::callback (uint32_t reqID, uint32_t addr)
{
    // Read data
    const DramRegisterWord *word =
        static_cast<const DramRegisterWord *>(GetRegister()->GetWord (addr));
    uint64_t data = word->value;
   
    bool intra_board;
    uint64_t val32;
    uint32_t destrhsid;
    uint8_t target_idx; 
    uint16_t val16;
    
    // Synapse type
    if(data & (1<<31))
    {
        intra_board = false;
        val32 = (data >> 21) & (0xffffffff);
        destrhsid = (data >> 18) & (0x7);
        target_idx  = (data >> 16) & (0x7);
        val16 = (data & (0x7fff));
    }
    else
    {
        SIM_ERROR ("Not Implemented. (Routing information)", GetFullName().c_str());
    }

   
    //outmsgs[PORT_data] = new DramMessage (destrhsid, val32, val16,
    //        intra_board, target_idx);
    if(dram_result == nullptr)
    {
        dram_result = new DramMessage (destrhsid, val32, val16,
                intra_board, target_idx);
    }
    else
    {
        SIM_ERROR ("Dram output is overwritten", GetFullName().c_str());
    }

    entry_cnt--;

    DEBUG_PRINT ("[DRAM] Receive dram request (reqID: %u, addr: %u)", reqID, addr);
    DEBUG_PRINT ("[DRAM] Send dram data to %d", destrhsid);
}




