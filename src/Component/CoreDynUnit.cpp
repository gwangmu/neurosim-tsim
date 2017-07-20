#include <Component/CoreDynUnit.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Register/MetaFileRegister.h>
#include <Register/MetaRegisterWord.h>
#include <Message/IndexMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/AxonMessage.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

CoreDynUnit::CoreDynUnit (string iname, Component *parent, 
        uint32_t num_neurons, uint32_t depth)
    : Module ("CoreDynUnitModule", iname, parent, depth)
{
    PORT_coreTS = CreatePort ("coreTS", Module::PORT_INPUT, 
            Prototype<SignalMessage>::Get());
    
    PORT_dynfin = CreatePort ("dynfin", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());
    PORT_axon = CreatePort ("axon", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());

    /* variable initialization */
    this->num_neurons_ = num_neurons;
    this->row_size_ = num_neurons;
    this->col_size_ = 64;
    
    pipeline_depth_ = depth;
    // NBC-SRAM-NBC-(NB(depth))-AMQ-AT-AMQ
    if(pipeline_depth_ > 15)
        SIM_ERROR ("Max pipeline depth is 15", 
                GetFullName().c_str());

    pipe_state_ = 0;
    pipe_mask_ = (1 << (pipeline_depth_-2)) - 2;
    
    spike_state_ = 0;
    spike_mask_ = (1 << pipeline_depth_) - 2;
    is_idle_ = true;
  
    ts_parity_ = false;
    idx_counter_ = num_neurons;

    // init script
    Register::Attr regattr (col_size_, row_size_);
    
    SetRegister (new MetaFileRegister (Register::SRAM, regattr));
    SetScript (new SpikeFileScript ());
}

void CoreDynUnit::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    SpikeInstruction *spk_inst = 
        static_cast<SpikeInstruction*>(instr);
    if (spk_inst)
    {
        INFO_PRINT ("[DYN] Spike instruction received");

        spike_trace_.clear();
        spike_trace_.assign 
            (spk_inst->spike_idx.begin(), 
             spk_inst->spike_idx.end());
    }
    
    // Process parity message
    SignalMessage *parity_msg = 
        static_cast<SignalMessage*>(inmsgs[PORT_coreTS]);
    if(parity_msg)
    {
        ts_parity_ = parity_msg->value;
        idx_counter_ = 0;
        
        INFO_PRINT("[DYN] Start Dynamics");
    }

    /* Dynamics Operation */
    pipe_state_ = (pipe_state_ << 1) & pipe_mask_;
    spike_state_ = (spike_state_ << 1) & spike_mask_;

    if (idx_counter_ != num_neurons_)
    {
        INFO_PRINT ("[DYN] Initiate %dth neuron dynamics", 
                idx_counter_);

        // Check spike
        bool is_spike =
            (spike_trace_.empty())? false : 
            (idx_counter_ == spike_trace_.front());
        if(is_spike)
        {
            spike_state_ |= 0x1;
            spike_trace_.pop_front();
            
            const MetaRegisterWord *word = 
                static_cast<const MetaRegisterWord *> 
                (GetRegister()->GetWord (idx_counter_));
            uint64_t metadata = word->value;

            uint64_t ax_addr = metadata >> 16;
            uint16_t ax_len = metadata & 0xffff;
            
            outmsgs[PORT_axon] = new AxonMessage (0, ax_addr, ax_len); 
        }
        else
        {
            // Push task in pipeline
            pipe_state_ |= 0x1;

        }

        idx_counter_++;
        if(idx_counter_ == num_neurons_)
            outmsgs[PORT_dynfin] = new SignalMessage (0, true);
    }


    // Update Idle state
    if (is_idle_ && (pipe_state_ || spike_state_))
    {
        is_idle_ = false;
        outmsgs[PORT_dynfin] = new SignalMessage (0, false); 
        INFO_PRINT ("[DYN] Start Dynamics");
    }
    else if (!is_idle_ && !(pipe_state_ || spike_state_))
    {
        is_idle_ = true;
        INFO_PRINT ("[DYN] End Dyanmics");

        GetScript()->NextSection();
    }
}





