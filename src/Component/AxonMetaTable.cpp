#include <Component/AxonMetaTable.h>

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
    this->col_size_ = 8; // bits

    this->read_n = 0;
    this->write_n = 0;

    this->counter = 0;    
}


void AxonMetaTable::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[RPORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;
        outmsgs[RPORT_data] = new AxonMessage (0, counter, 1);
        counter++;

        DEBUG_PRINT("[SRAM] Receive read request, and send message");
    }
}
