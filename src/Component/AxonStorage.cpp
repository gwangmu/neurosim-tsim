#include <Component/AxonStorage.h>

#include <Message/IndexMessage.h>
#include <Message/DramMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

AxonStorage::AxonStorage (string iname, Component* parent)
    : Module ("AxonStorage", iname, parent, 1)
{

    PORT_addr = CreatePort ("r_addr", Module::PORT_INPUT,
            Prototype<IndexMessage>::Get());
    PORT_data = CreatePort ("r_data", Module::PORT_OUTPUT,
            Prototype<DramMessage>::Get());
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    entry_cnt = 0;
    is_idle_ = true;

    is_request = false;
    delay_counter = 0;
    this->counter = 0;
    delay = 2;
}


void AxonStorage::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /* Read */
    IndexMessage *raddr_msg = static_cast<IndexMessage*>(inmsgs[PORT_addr]);

    if (is_request)
    {
        delay_counter--;
        if(delay_counter == 0)
        {
            DEBUG_PRINT ("[DRAM] Response to read request");

            outmsgs[PORT_data] = new DramMessage (0, counter, 0, false, 0);
            is_request = false;
            entry_cnt--;
            counter++;
        }
    }
    else if(raddr_msg)
    {
        uint32_t read_addr = raddr_msg->value;
        entry_cnt++;

        delay_counter = delay;
        is_request = true;

        DEBUG_PRINT("[DRAM] Receive read request");
    }

    if(!is_idle_ && (entry_cnt==0))
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
}



