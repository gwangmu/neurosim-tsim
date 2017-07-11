#include <Component/NeuronBlock.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/NeuronBlockMessage.h>
#include <Message/IndexMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>

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

    OPORT_waddr = CreatePort ("w_addr", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    OPORT_wdata = CreatePort ("w_data", Module::PORT_OUTPUT,
            Prototype<StateMessage>::Get());
   
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    // init script
    SetScript (new SpikeFileScript ());

    /* variable initialization */
    pipeline_depth_ = depth;
    
    idle_time_ = 0;
    is_idle_ = true;
}

void NeuronBlock::Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr)
{
    NeuronBlockInMessage *in_msg = static_cast<NeuronBlockInMessage*>(inmsgs[PORT_in]);
    SpikeInstruction *spk_inst = static_cast<SpikeInstruction*>(instr);

    if (spk_inst)
    {
        DEBUG_PRINT ("[NB] Spike instruction received");

        spike_trace_.clear();
        spike_trace_.assign (spk_inst->spike_idx.begin(), spk_inst->spike_idx.end());

        DEBUG_PRINT ("SPIKE TRACE - len %zu %zu", spk_inst->spike_idx.size(), spike_trace_.size());
    }

    // Add job in pipeline
    if(in_msg)
    {
        uint32_t neuron_idx = in_msg->idx;
        
        bool is_spike = (spike_trace_.empty())? false : (neuron_idx == spike_trace_.front());
        if(is_spike)
            spike_trace_.pop_front();
        

        outmsgs[PORT_out] = new NeuronBlockOutMessage (0, neuron_idx, is_spike);
        DEBUG_PRINT ("%p", outmsgs[PORT_out]);
        outmsgs[OPORT_waddr] = new IndexMessage (0, neuron_idx);
        
        State s = 0;
        outmsgs[OPORT_wdata] = new IndexMessage (0, s);

        DEBUG_PRINT ("[NB] Start %uth neuron dynamics", neuron_idx);
        
        if(is_idle_)
            outmsgs[OPORT_idle] = new SignalMessage (0, false);


        idle_time_ = 0;
        is_idle_ = false;
    }
    else if (!is_idle_)
    {
        if(idle_time_ == pipeline_depth_)
        {
            outmsgs[OPORT_idle] = new SignalMessage (0, true);
            is_idle_ = true;
            GetScript()->NextSection();

            DEBUG_PRINT ("[NB] NB is idle. Process next section");
        }
        
        idle_time_++;
    }
   

}





