#include <Component/FastDelayMgr.h>

#include <Message/AxonMessage.h>
#include <Message/DelayMetaMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <random>

USING_TESTBENCH;

FastDelayMgr::FastDelayMgr (string iname, Component* parent, uint8_t board_idx)
    : Module ("FastDelayMgr", iname, parent, 1)
{
    PORT_input = CreatePort ("Input", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    PORT_TSparity = CreatePort ("TSParity", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    PORT_output = CreatePort ("Output", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    PORT_idle = CreatePort ("Idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    num_delay_ = GET_PARAMETER (num_delay);
    min_delay_ = GET_PARAMETER (min_delay);

    delayed_spks_.clear();
    //delayed_spks_.resize(num_delay_);
 
    uint16_t num_boards = GET_PARAMETER (num_boards);

    this->board_idx_ = board_idx;
    this->num_neurons_ = GET_PARAMETER (num_neurons);
    this->neurons_per_board_ = num_neurons_ / num_boards;

    this->avg_syns_ = GET_PARAMETER (avg_synapses);
    board_syns_ = avg_syns_ * num_delay_ * (neurons_per_board_ / 4);
   
    SetScript (new SpikeFileScript());

    state_ = IDLE;
    is_idle_ = false;
    fetch_fin_ = true;
    ts_parity_ = false;

    // Pseudo-random
    // table
    uint32_t n = GET_PARAMETER(num_samples);
    uint32_t pint = GET_PARAMETER(probability);
    double p = pint / double(1000000);
    
    std::default_random_engine generator;
    std::binomial_distribution<int> distribution (n, p); 
    for (int i=0; i<256; i++)
    {
        synlen_table[i] = distribution(generator);
    }

    input_n = 0;
}

void FastDelayMgr::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
{
    SpikeInstruction *spk_inst =
        static_cast<SpikeInstruction*> (instr);
    if (spk_inst && num_delay_ != 0)
    {
        if(spk_inst->is_inh)
            delayed_spks_.push_back (DelayedSpk(1));
        else      
            delayed_spks_.push_back (DelayedSpk(num_delay_));

        delayed_spks_.back().spikes.assign 
            (spk_inst->spike_idx.begin(),
             spk_inst->spike_idx.end());
        delayed_spks_.back().is_inh = spk_inst->is_inh;

        PRINT("Spike len %u", spk_inst->spk_len);
    }

    IntegerMessage *ts_msg = 
        static_cast<IntegerMessage*> (inmsgs[PORT_TSparity]);
    if(ts_msg)
    {
        if(ts_msg->value != ts_parity_)
        {
            INFO_PRINT ("[FDM] Get TS parity");
            fetch_fin_ = false;
            spk_idx_ = 0;
            delay_it_ = delayed_spks_.begin();
            ts_parity_ = ts_msg->value;
            input_n = 0;
        
            GetScript()->NextSection();
        }
    }


    AxonMessage *input_msg =
        static_cast<AxonMessage*> (inmsgs[PORT_input]);
    if(state_counter_ != 0)
    {
        inmsgs[PORT_input] = nullptr;
        state_counter_--;
    }    
    else if(input_msg)
    {
        // Insert
        state_ = INSERT;
        state_counter_ = 2;
        INFO_PRINT ("[FDM] Insert state %d/%d", 
                GetInQueSize (PORT_input), ++input_n);
    }
    else
    {
        // Fetch. Fast-mode support only continuous delay
        if(!fetch_fin_)
        {
            state_ = FETCH;
            state_counter_ = 2;            
        }
        else if(!is_idle_)
        {
            INFO_PRINT ("[FDM] state counter %d (fin %d)", state_counter_, fetch_fin_);
            outmsgs[PORT_idle] = new IntegerMessage (1);
            is_idle_ = true;
        }
    }

    if(state_counter_ != 0 && is_idle_)
    {
        INFO_PRINT ("[FDM] state counter %d (fin %d)", state_counter_, fetch_fin_);
        delete outmsgs[PORT_idle];
        outmsgs[PORT_idle] = new IntegerMessage (0);
        is_idle_ = false;
    }

    if(state_ == FETCH && !state_counter_)
    {
        if(delay_it_ == delayed_spks_.end() ||
                delayed_spks_.empty())
        {
            fetch_fin_ = true;
            state_counter_ = 0;
        }
        else
        {

            uint32_t out_idx = (*delay_it_).spikes[spk_idx_];
            // Send Message
            bool is_boardmsg = (out_idx / num_neurons_) == board_idx_;
            uint64_t ax_addr;
            uint16_t ax_len;

            uint16_t delay = (*delay_it_).delay;
            if(is_boardmsg) // Intra-spike
            {
                uint32_t sidx = out_idx % (num_neurons_ / 4);
                
                ax_addr = avg_syns_ * num_delay_ * sidx + 
                            avg_syns_ * (num_delay_ - delay);
                ax_len = synlen_table[(sidx + delay)%256];
            }
            else // Inter-spike
            {
                uint32_t bidx = out_idx / num_neurons_;
                bidx = (bidx > board_idx_)? bidx-1 : bidx;

                uint32_t sidx = out_idx % (num_neurons_ / 4);
               
                ax_addr = base_addr_ + bidx * board_syns_;
                ax_addr += avg_syns_ * num_delay_ * sidx + 
                            avg_syns_ * (num_delay_ - delay);
                ax_len = synlen_table[(sidx + delay)%256];

            }

            outmsgs[PORT_output] = 
                new AxonMessage (0, ax_addr, ax_len);

            // Advance 
            spk_idx_ += 1;
            if(spk_idx_ == (*delay_it_).spikes.size())
            {
                (*delay_it_).delay--;
                if(!(*delay_it_).delay)
                {
                    (*delay_it_).spikes.clear();
                    delay_it_ = delayed_spks_.erase (delay_it_);
                }
                else
                    delay_it_ ++;

                spk_idx_ = 0;
            }
        }
    }
}
