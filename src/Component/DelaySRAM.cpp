#include <Component/DelaySRAM.h>

#include <Register/DelayFileRegister.h>
#include <Register/DelayRegisterWord.h>

#include <Message/IndexMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/DelayMetaMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

DelaySRAM::DelaySRAM (string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : Module ("DelaySRAM", iname, parent, 1)
{
    this->row_size_ = row_size;
    this->col_size_ = col_size;

    read_n = 0;
    write_n = 0;
    
    RPORT_addr = CreatePort ("raddr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    RPORT_data = CreatePort ("rdata", Module::PORT_OUTPUT,
            Prototype<DelayMetaMessage>::Get());

    WPORT_addr = CreatePort ("waddr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    WPORT_data = CreatePort ("wdata", Module::PORT_INPUT,
            Prototype<DelayMetaMessage>::Get());

    // Register::Attr regattr (col_size_, row_size_);
    // SetRegister (new DelayFileRegister (Register::SRAM, regattr));
}


void DelaySRAM::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[RPORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;
        
        if(read_addr == 0)
        {
            SIM_FATAL ("Delay SRAM doesn't have address zero",
                    GetName().c_str());
            return;
        }

        // const DelayRegisterWord *word =
        //     static_cast<const DelayRegisterWord*>
        //         (GetRegister()->GetWord(read_addr));
        // if(word)
        // {
        //     uint32_t next_addr = word->next_addr;
        //     uint64_t addr_sub = word->addr_sub;
        //     uint16_t val16 = word->val16;

        //     outmsgs[RPORT_data] = new DelayMetaMessage (0, next_addr,
        //                                             addr_sub, val16);

        //     INFO_PRINT ("[DeSRAM] read address %lx - %x %lx %u", 
        //                     read_addr, next_addr, addr_sub, val16);
        // }
        // else
        {
            outmsgs[RPORT_data] = new DelayMetaMessage (0, 0, 0, 0);
            INFO_PRINT ("[DeSRAM] read null data (address %x)",
                        read_addr);
        }
        
    }


    /* Write */
    IndexMessage *waddr_msg = static_cast<IndexMessage*>(inmsgs[WPORT_addr]);
    DelayMetaMessage *wdata_msg = static_cast<DelayMetaMessage*>(inmsgs[WPORT_data]);

    if(waddr_msg && wdata_msg)
    {
        uint32_t write_addr = waddr_msg->value;
        
        INFO_PRINT("[DeSRAM] Receive write request (%x - %x %lx %u)",
                    write_addr,
                    wdata_msg->next_addr, 
                    wdata_msg->addr_sub,
                    wdata_msg->val16);
        
        // GetRegister()->SetWord (write_addr, 
        //                     new DelayRegisterWord (wdata_msg->next_addr,
        //                                            wdata_msg->addr_sub,
        //                                            wdata_msg->val16));
    }
    else if (unlikely (waddr_msg || wdata_msg))
    {
        SIM_ERROR ("SRAM receive only either an addr or data", GetFullName().c_str()); 
        return;
    }
}
