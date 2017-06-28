#include <Component/NeuronBlock.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/NeuronBlockMessage.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

NeuronBlock::NeuronBlock (string iname, Component *parent, uint32_t depth)
    : Module ("NeuronBlockModule", iname, parent, depth)
{
    //IPORT_Nidx = CreatePort ("Nidx_in", Module::PORT_INPUT, 
    //        Prototype<IndexMessage>::Get());
    //IPORT_State = CreatePort ("State_in", Module::PORT_INPUT, 
    //        Prototype<StateMessage>::Get());
    //IPORT_DeltaG = CreatePort ("DeltaG_in", Module::PORT_INPUT, 
    //        Prototype<DeltaGMessage>::Get());
    //OPORT_Nidx = CreatePort ("Nidx_out", Module::PORT_OUTPUT, 
    //        Prototype<IndexMessage>::Get());
    //OPORT_Spike = CreatePort ("Spike_out", Module::PORT_OUTPUT, 
    //        Prototype<SignalMessage>::Get());

    PORT_in = CreatePort ("NeuronBlock_in", Module::PORT_INPUT, 
            Prototype<NeuronBlockInMessage>::Get());
    PORT_out = CreatePort ("NeuronBlock_out", Module::PORT_OUTPUT, 
            Prototype<NeuronBlockOutMessage>::Get());


    
    // init script
    SetScript (new SpikeFileScript ());

    /* variable initialization */
    pipeline_depth_ = depth;
}

void NeuronBlock::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // IndexMessage *idx_msg = static_cast<IndexMessage*>(inmsgs[IPORT_Nidx]);
    // StateMessage *state_msg = static_cast<StateMessage*>(inmsgs[IPORT_State]);
    // DeltaGMessage *deltaG_msg = static_cast<DeltaGMessage*>(inmsgs[IPORT_DeltaG]);

    NeuronBlockInMessage *in_msg = static_cast<NeuronBlockInMessage*>(inmsgs[PORT_in]);

    SpikeInstruction *spk_inst = static_cast<SpikeInstruction*>(instr);

    if (spk_inst)
    {
        DEBUG_PRINT ("instruction received");
        std::vector<int> v = {2, 3, 3};
        std::vector<int> f = {2, 3, 3};

        spike_trace_.clear();
        std::move( spk_inst->spike_idx.begin(),
                    spk_inst->spike_idx.end(),
                    spike_trace_.begin()
                 );
    }
    
    // Complete Neuronal Dynamics
    if(pipelined_idx_.size() == pipeline_depth_)
    {
        uint32_t pipe_head = pipelined_idx_.front();
        pipelined_idx_.pop_front();
        
        bool is_spike = (pipe_head == spike_trace_.front());
        if(is_spike)
            spike_trace_.pop_front();

        outmsgs[PORT_out] = new NeuronBlockOutMessage (0, pipe_head, is_spike);
        
        DEBUG_PRINT ("dynamics completed (idx: %d, spike: %d)",
                pipe_head, is_spike);
    }

    // Add job in pipeline
    if(in_msg)
    {
        uint32_t neuron_idx = in_msg->idx;
        pipelined_idx_.push_back(neuron_idx);

        DEBUG_PRINT ("start %uth neuron dynamics", neuron_idx);
    }
}
