#include <Component/AxonStorage.h>

#include <Message/IndexMessage.h>
#include <Message/DramMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

AxonStorage::AxonStorage (string iname, Component* parent)
    : Module ("AxonStorage", iname, parent, 1)
{

    PORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    PORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<DramMessage>::Get());

    this->counter = 0;    
}


void AxonStorage::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_addr]);

    if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;
        outmsgs[PORT_data] = new DramMessage (0, 0, 0, false, 0);
        counter++;

        DEBUG_PRINT("[DRAM] Receive read request, and send message");
    }
}
