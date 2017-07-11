#include <Component/AxonTransmitter.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonTransmitter::AxonTransmitter (string iname, Component *parent)
    : Module ("AxonTransmitter", iname, parent, 1)
{
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Axon = CreatePort ("axon_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    is_idle_ = false;
}

void AxonTransmitter::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg)
    {
        DEBUG_PRINT ("[AT] Receive axon message (addr %lu)", axon_msg->value) 
        outmsgs[OPORT_Axon] = new AxonMessage (0, axon_msg->value, axon_msg->len); 
        
        if (is_idle_)
        {
            DEBUG_PRINT ("[AT] Axon metadata transmitter is busy");
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
        }
    }
    else if(!is_idle_ && *outque_size==0)
    {
        DEBUG_PRINT ("[AT] Axon metadata transmitter is idle");
        is_idle_ = true;
        outmsgs[OPORT_idle] = new IntegerMessage (1);
    }
}
