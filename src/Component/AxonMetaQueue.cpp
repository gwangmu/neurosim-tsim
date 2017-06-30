#include <Component/AxonMetaQueue.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/IndexMessage.h>
#include <Message/AxonMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonMetaQueue::AxonMetaQueue (string iname, Component *parent)
    : Module ("AxonMetaQueue", iname, parent, 1)
{
    IPORT_NB = CreatePort ("nb", Module::PORT_INPUT,
            Prototype<NeuronBlockOutMessage>::Get());
    IPORT_Meta = CreatePort ("meta", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    IPORT_Core_sel = CreatePort ("core_sel", Module::PORT_INPUT,
            Prototype<SelectMessage>::Get());

    OPORT_SRAM = CreatePort ("sram", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    OPORT_Axon = CreatePort ("axon", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Empty = CreatePort ("empty", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
}

void AxonMetaQueue::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    NeuronBlockOutMessage *nb_msg = static_cast<NeuronBlockOutMessage*>(inmsgs[IPORT_NB]);
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Meta]);
    SelectMessage *sel_msg = static_cast<SelectMessage*>(inmsgs[IPORT_Core_sel]);

    if(nb_msg)
    {
        uint32_t idx = nb_msg->value;
        bool spike = nb_msg->spike;
    
        if(spike)
        {
            DEBUG_PRINT ("[AMQ] Receive spike. Send read request to Axon Metadata Table");
            outmsgs[OPORT_SRAM] = new IndexMessage (0, idx);
        }
    }

    if(axon_msg)
    {
        uint64_t ax_addr = axon_msg->value;
        uint16_t ax_len = axon_msg->len;

        outmsgs[OPORT_Axon] = new AxonMessage (0, ax_addr, ax_len);
    }

    if(sel_msg)
    {

    }



}
