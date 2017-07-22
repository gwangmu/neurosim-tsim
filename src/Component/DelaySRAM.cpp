#include <Component/DelaySRAM.h>

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

}


void DelaySRAM::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
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

        uint64_t next_addr = 1;
        uint16_t addr_sub = 2;
        uint16_t val16 = 3;

        outmsgs[RPORT_data] = new DelayMetaMessage (0, next_addr,
                                                    addr_sub, val16);

        INFO_PRINT("[DMSRAM] Receive read request, and send message");
    }


    /* Write */
    IndexMessage *waddr_msg = static_cast<IndexMessage*>(inmsgs[WPORT_addr]);
    DelayMetaMessage *wdata_msg = static_cast<DelayMetaMessage*>(inmsgs[WPORT_data]);

    if(waddr_msg && wdata_msg)
    {
        INFO_PRINT("[DMSRAM] Receive write request");
        uint32_t write_addr = waddr_msg->value;
    }
    else if (unlikely (waddr_msg || wdata_msg))
    {
        SIM_ERROR ("SRAM receive only either an addr or data", GetFullName().c_str()); 
        return;
    }
}
