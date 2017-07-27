#include <Component/AxonMetaRecv.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonMetaRecv::AxonMetaRecv (string iname, Component *parent)
    : Module ("AxonMetaRecv", iname, parent, 1)
{
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Axon = CreatePort ("axon_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_delay = CreatePort ("delay_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    is_idle_ = false;
}

void AxonMetaRecv::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg)
    {
        INFO_PRINT ("[AMR] Receive message (addr %lu)", axon_msg->value);

        if(!axon_msg->delay)
            outmsgs[OPORT_Axon] = 
                new AxonMessage (0, axon_msg->value, axon_msg->len);
        else    
            outmsgs[OPORT_delay] = 
                new AxonMessage (0, axon_msg->value, 
                                axon_msg->len, axon_msg->delay);

        if (is_idle_)
        {
            INFO_PRINT ("[AMR] Axon metadata receiver is busy");
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
        }
    }
    else if(!is_idle_)
    {
        INFO_PRINT ("[AMR] Axon metadata receiver is idle");
        is_idle_ = true;
        outmsgs[OPORT_idle] = new IntegerMessage (1);
    }
}
