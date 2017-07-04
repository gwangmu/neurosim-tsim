#include <Component/DeltaSRAM.h>

#include <Message/IndexMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/SignalMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

DeltaStorage::DeltaStorage (string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : Module ("DeltaStorage", iname, parent, 1)
{

    NPORT_raddr = CreatePort ("nr_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    NPORT_rdata = CreatePort ("nr_data", Module::PORT_OUTPUT,
            Prototype<DeltaGMessage>::Get());
    APORT_raddr = CreatePort ("ar_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    APORT_rdata = CreatePort ("ar_data", Module::PORT_OUTPUT,
            Prototype<DeltaGMessage>::Get());

    PORT_waddr = CreatePort ("w_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    PORT_wdata = CreatePort ("w_data", Module::PORT_INPUT,
            Prototype<DeltaGMessage>::Get());

    PORT_TSparity = CreatePort ("TSparity", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());

    cur_parity = false;

    this->row_size_ = row_size;
    this->col_size_ = col_size;
}   


void DeltaStorage::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[NPORT_raddr]);
    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;

        int64_t value = 0;
        outmsgs[NPORT_rdata] = new DeltaGMessage (0, value);

        DEBUG_PRINT("[DGSRAM] Receive read request, and send message");
    }
    
    raddr_msg = static_cast<IndexMessage*>(inmsgs[APORT_raddr]);
    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;

        int64_t value = 0;
        outmsgs[APORT_rdata] = new DeltaGMessage (0, value);

        DEBUG_PRINT("[DGSRAM] Receive read request, and send message");
    }


    /* Write */
    IndexMessage *waddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_waddr]);
    DeltaGMessage *wdata_msg = static_cast<DeltaGMessage*>(inmsgs[PORT_wdata]);

    if(waddr_msg && wdata_msg)
    {
        DEBUG_PRINT("[DGSRAM] Receive write request");
        uint32_t write_addr = waddr_msg->value;
    }
    else if (unlikely (waddr_msg || wdata_msg))
    {
        SIM_ERROR ("SRAM receive only either an addr or data", GetFullName().c_str()); 
        return;
    }

    /* reset */
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[PORT_TSparity]);
    if(parity_msg)
    {
        cur_parity = parity_msg->value;
        {
            // Reset current parity SRAM
            DEBUG_PRINT ("[DGSRAM] Reset SRAM");
        }
    }
}
