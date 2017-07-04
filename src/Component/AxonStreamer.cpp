#include <Component/AxonStreamer.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/AxonMessage.h>
#include <Message/IndexMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonStreamer::AxonStreamer (string iname, Component *parent)
    : Module ("AxonStreamer", iname, parent, 1)
{
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Addr = CreatePort ("addr_out", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());

    is_idle_ = true;

    // DRAM Spec.
    read_bytes = 64; 
}

void AxonStreamer::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(is_idle_ && axon_msg)
    {
        base_addr_ = axon_msg->value;
        ax_len_ = axon_msg->len;
        read_addr_ = base_addr_;
        is_idle_ = false;        
        
        DEBUG_PRINT("[AS] Start DRAM streaming (addr: %u, len: %u)", base_addr_, ax_len_);
    }
    else if(!is_idle_ && axon_msg)
        inmsgs[IPORT_Axon] = nullptr;

    if(!is_idle_)
    {
        DEBUG_PRINT ("[AS] Send read request");
        outmsgs[OPORT_Addr] = new IndexMessage (0, read_addr_);
        
        read_addr_ += read_bytes;

        if(read_addr_ >= base_addr_ + ax_len_)
        {
            DEBUG_PRINT ("[AS] Finish DRAM streaming (addr: %u, len: %u)", base_addr_, ax_len_);
            is_idle_ = true;
        }
    }
}
