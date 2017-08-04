#include <Component/CoreDynUnit.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Register/EmptyRegister.h>
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
#include <random>

using namespace std;

USING_TESTBENCH;

CoreDynUnit::CoreDynUnit (string iname, Component *parent, 
        uint32_t num_neurons, uint32_t depth, uint16_t core_idx)
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
   
    this->avg_synapses_ = GET_PARAMETER(avg_synapses);
    this->min_delay_ = GET_PARAMETER(min_delay);
    uint16_t num_delay = GET_PARAMETER(num_delay);

    this->dyn_latency_ = GET_PARAMETER(dyn_latency);
    dyn_counter_ = dyn_latency_;

    avg_synapses_ *= num_delay; // Connections per neurons

    // Propagator Information
    this->core_idx_ = core_idx;

    uint16_t num_cores = GET_PARAMETER(num_cores);
    uint16_t num_chips = GET_PARAMETER(num_chips);
    uint16_t num_propagators = GET_PARAMETER(num_propagators);
    
    uint8_t group_size = num_cores * num_chips / num_propagators;
    
    prop_idx_ = core_idx / group_size;
    base_addr_ = num_neurons_ * avg_synapses_ * 
                (core_idx_ % group_size);

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
    is_finish_ = true;

    ts_parity_ = false;
    idx_counter_ = num_neurons;

    // init script
    Register::Attr regattr (col_size_, row_size_);
    SetRegister (new EmptyRegister (Register::SRAM, regattr));
    
    SetScript (new SpikeFileScript ());

    // Construct pseudo-random table
    uint32_t n = GET_PARAMETER(num_samples);
    uint32_t pint = GET_PARAMETER(probability);
    double p = pint / double(1000000);

    std::default_random_engine generator;
    std::binomial_distribution<int> distribution (n, p); 
    for (int i=0; i<1024; i++)
    {
        synlen_table[i] = distribution(generator);
    }
}

void CoreDynUnit::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
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
        if(ts_parity_ != parity_msg->value)
        {
            INFO_PRINT("[DYN] Start Dynamics %d/%d",
                    ts_parity_, parity_msg->value);
            
            ts_parity_ = parity_msg->value;
            idx_counter_ = 0;
            
        }
    }

    /* Dynamics Operation */
    pipe_state_ = (pipe_state_ << 1) & pipe_mask_;
    spike_state_ = (spike_state_ << 1) & spike_mask_;

    if (idx_counter_ != num_neurons_)
    {
        if(dyn_counter_)
        {
            dyn_counter_--;
        }
        else
        {
            //INFO_PRINT ("[DYN] Initiate %d/%d neuron dynamics", 
            //        idx_counter_, num_neurons_);

            // Check spike
            bool is_spike =
                (spike_trace_.empty())? false : 
                (idx_counter_ == spike_trace_.front());
            if(is_spike)
            {
                spike_state_ |= 0x1;
                spike_trace_.pop_front();

                // const MetaRegisterWord *word = 
                //     static_cast<const MetaRegisterWord *> 
                //     (GetRegister()->GetWord (idx_counter_));

                // uint64_t metadata = word->value;

                // uint16_t delay = (metadata >> 52) & 0x3ff;
                // uint64_t ax_addr = (metadata >> 16) & 0xfffffffff;
                // uint16_t ax_len = metadata & 0xffff;

                GetRegister()->GetWord (idx_counter_);

                uint16_t delay = min_delay_;
                uint64_t ax_addr = base_addr_ + avg_synapses_ * idx_counter_;
                uint16_t ax_len = synlen_table[idx_counter_ % 1024];
                
                bool is_inh = idx_counter_ > 0.8*num_neurons_;
                outmsgs[PORT_axon] = new AxonMessage (0, ax_addr, 
                        ax_len, delay, prop_idx_, is_inh);

                INFO_PRINT ("[DYN] Send Axon Message (addr %lu, len %u)",
                        ax_addr, ax_len);
            }
            else
            {
                // Push task in pipeline
                pipe_state_ |= 0x1;

            }

            idx_counter_++;
            dyn_counter_ = dyn_latency_;

            if(idx_counter_ == num_neurons_)
            {
                INFO_PRINT ("[DYN] Dynamics finish");
                is_finish_ = true;
                outmsgs[PORT_dynfin] = new SignalMessage (0, true);
            }
        }
    }


    // Update Idle state
    if (is_idle_ && (pipe_state_ || spike_state_) && is_finish_)
    {
        is_idle_ = false;
        is_finish_ = false;
        outmsgs[PORT_dynfin] = new SignalMessage (0, false); 
        INFO_PRINT ("[DYN] Start Dynamics");
    }
    else if (!is_idle_ && !(pipe_state_ || spike_state_) && is_finish_)
    {
        is_idle_ = true;
        INFO_PRINT ("[DYN] End Dyanmics");

        GetScript()->NextSection();
    }
}





