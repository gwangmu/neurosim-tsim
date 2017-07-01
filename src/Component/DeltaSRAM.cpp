#include <Component/DeltaSRAM.h>

#include <Message/IndexMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/SignalMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

DeltaSRAM::DeltaSRAM (string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : SRAMModule ("DeltaSRAM", iname, parent, row_size, col_size)
{

     RPORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
             Prototype<IndexMessage>::Get());
     RPORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
             Prototype<DeltaGMessage>::Get());

     WPORT_addr = CreatePort ("w_addr", Module::PORT_INPUT,
             Prototype<IndexMessage>::Get());
     WPORT_data = CreatePort ("w_data", Module::PORT_INPUT,
             Prototype<DeltaGMessage>::Get());

     PORT_reset = CreatePort ("reset", Module::PORT_INPUT,
             Prototype<SignalMessage>::Get());
}


void DeltaSRAM::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[RPORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;

        int64_t value = 0;
        outmsgs[RPORT_data] = new DeltaGMessage (0, value);

        DEBUG_PRINT("[SRAM] Receive read request, and send message");
    }


    /* Write */
    IndexMessage *waddr_msg = static_cast<IndexMessage*>(inmsgs[WPORT_addr]);
    DeltaGMessage *wdata_msg = static_cast<DeltaGMessage*>(inmsgs[WPORT_data]);

    if(waddr_msg && wdata_msg)
    {
        DEBUG_PRINT("[SRAM] Receive write request");
        uint32_t write_addr = waddr_msg->value;
    }
    else if (unlikely (waddr_msg || wdata_msg))
    {
        SIM_ERROR ("SRAM receive only either an addr or data", GetFullName().c_str()); 
        return;
    }

    /* reset */
    SignalMessage *reset_msg = static_cast<SignalMessage*>(inmsgs[PORT_reset]);
    if(reset_msg)
    {
        if(reset_msg->value)
        {
            DEBUG_PRINT("[DeltaG SRAM] reset SRAM");
        }
    }
}

DeltaStorage::DeltaStorage (string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : Component ("DeltaStorage", iname, parent)
{
    Module *odd_sram = new DeltaSRAM ("odd_sram", this, row_size, col_size);
    Module *even_sram = new DeltaSRAM ("even_sram", this, row_size, col_size);

    ExportPort ("ORAddr", odd_sram, "r_addr");
    ExportPort ("ORData", odd_sram, "r_data");
    ExportPort ("OWAddr", odd_sram, "w_addr");
    ExportPort ("OWData", odd_sram, "w_data");
    ExportPort ("Oreset", odd_sram, "reset");
    
    ExportPort ("ERAddr", even_sram, "r_addr");
    ExportPort ("ERData", even_sram, "r_data");
    ExportPort ("EWAddr", even_sram, "w_addr");
    ExportPort ("EWData", even_sram, "w_data");
    ExportPort ("Ereset", even_sram, "reset");
}
