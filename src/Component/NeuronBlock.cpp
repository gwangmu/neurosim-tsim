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
    PORT_in = CreatePort ("NeuronBlock_in", Module::PORT_INPUT, 
            Prototype<NeuronBlockInMessage>::Get());
    PORT_out = CreatePort ("NeuronBlock_out", Module::PORT_OUTPUT, 
            Prototype<NeuronBlockOutMessage>::Get());


    
    // init script
    SetScript (new SpikeFileScript ());

    /* variable initialization */
    pipeline_depth_ = depth;
}

void NeuronBlock::Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr)
{
    NeuronBlockInMessage *in_msg = static_cast<NeuronBlockInMessage*>(inmsgs[PORT_in]);

    SpikeInstruction *spk_inst = static_cast<SpikeInstruction*>(instr);

    if (spk_inst)
    {
        DEBUG_PRINT ("instruction received");

        spike_trace_.clear();
        spike_trace_.assign (spk_inst->spike_idx.begin(), spk_inst->spike_idx.end());

        // std::copy( spk_inst->spike_idx.begin(),
        //             spk_inst->spike_idx.end(),
        //             spike_trace_.begin()
        //          );

        DEBUG_PRINT ("SPIKE TRACE - len %zu %zu", spk_inst->spike_idx.size(), spike_trace_.size());
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
