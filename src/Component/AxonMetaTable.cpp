#include <Component/AxonMetaTable.h>

#include <Register/MetaFileRegister.h>
#include <Register/MetaRegisterWord.h>
#include <Message/IndexMessage.h>
#include <Message/AxonMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

AxonMetaTable::AxonMetaTable (string iname, Component* parent,
        uint32_t row_size)
    : Module ("AxonMetaTable", iname, parent, 1)
{

    RPORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    RPORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());

    this->row_size_ = row_size;
    this->col_size_ = 64; // bits

    this->read_n = 0;
    this->write_n = 0;

    Register::Attr regattr (col_size_, row_size_);
    SetRegister (new MetaFileRegister (Register::SRAM, regattr));
}


void AxonMetaTable::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[RPORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;

        const MetaRegisterWord *word = 
            static_cast<const MetaRegisterWord *>(GetRegister()->GetWord (read_addr));
        uint64_t metadata = word->value;

        uint64_t dram_addr = metadata >> 16;
        uint16_t len = metadata & 0xffff; 

        INFO_PRINT("[AMT] Receive read request (idx: %u), \
                and send message (addr: %lu, len %d)",
                read_addr, dram_addr, len);
        
        outmsgs[RPORT_data] = new AxonMessage (0, dram_addr, len);
    }
}
