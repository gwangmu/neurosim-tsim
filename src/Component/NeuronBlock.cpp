#include <Component/NeuronBlock.h>
#include <Message/IndexMessage.h>
#include <Message/StateMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/SignalMessage.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

NeuronBlock::NeuronBlock (string iname, Component *parent, uint32_t depth)
    : Module ("NeuronBlockModule", iname, parent)
{
    IPORT_Nidx = CreatePort ("Nidx_in", Module::PORT_INPUT, 
            Prototype<IndexMessage>::Get());
    IPORT_State = CreatePort ("State_in", Module::PORT_INPUT, 
            Prototype<StateMessage>::Get());
    IPORT_DeltaG = CreatePort ("DeltaG_in", Module::PORT_INPUT, 
            Prototype<DeltaGMessage>::Get());

    OPORT_Nidx = CreatePort ("Nidx_out", Module::PORT_OUTPUT, 
            Prototype<IndexMessage>::Get());
    OPORT_Spike = CreatePort ("Spike_out", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());

    pipeline_depth_ = depth;
    pipeline_state_ = 0;

    mask_ = (1 << depth) - 1;

    // DEBUG
    for(int i=0; i<10; i++)
    {
        for(int j=0; j<10; j++)
        {
            spike_traces_.push_back(std::vector<int>());
            auto traces = &spike_traces_.back();

            traces->push_back(j);
        }
    }
}

void NeuronBlock::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    IndexMessage *idx_msg = static_cast<IndexMessage*>(inmsgs[IPORT_Nidx]);
    StateMessage *state_msg = static_cast<StateMessage*>(inmsgs[IPORT_State]);
    DeltaGMessage *deltaG_msg = static_cast<DeltaGMessage*>(inmsgs[IPORT_DeltaG]);

    // Advance pipeline
    pipeline_state_ = (pipeline_state_ << 1) & mask_; 
    
    if (idx_msg)
    {
        pipeline_state_ |= 0x1;
        DEBUG_PRINT ("state = %x", pipeline_state_);
    }


}
