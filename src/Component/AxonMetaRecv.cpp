#include <Component/AxonMetaRecv.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

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
}

void AxonMetaRecv::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg)
    {
        DEBUG_PRINT ("[AMR] Receive message (addr %lu)", axon_msg->value);
        outmsgs[OPORT_Axon] = new AxonMessage (0, axon_msg->value, axon_msg->len); 
    }
}
