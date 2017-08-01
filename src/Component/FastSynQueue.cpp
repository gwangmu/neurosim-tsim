#include <Component/FastSynQueue.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/SignalMessage.h>
#include <Message/SynapseMessage.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

FastSynQueue::FastSynQueue (
        string iname, Component *parent, uint32_t num_propagators)
    : Module ("FastSynQueueModule", iname, parent, 1)
{
    SetClock("dram");
    PORT_coreTS = CreatePort ("coreTS", Module::PORT_INPUT, 
            Prototype<SignalMessage>::Get());
    PORT_empty = CreatePort ("empty", Module::PORT_OUTPUT, 
            Prototype<IntegerMessage>::Get());

    this->num_propagators_ = num_propagators;
    for(int i=0; i<num_propagators_; i++)
    {
        PORT_syns.push_back(
                CreatePort ("syn" + to_string(i), Module::PORT_INPUT,
                    Prototype<SynapseMessage>::Get()));
        PORT_synTS.push_back(
                CreatePort ("synTS" + to_string(i), Module::PORT_INPUT,
                    Prototype<SignalMessage>::Get()));
        PORT_acc.push_back (
                CreatePort ("acc" + to_string(i), Module::PORT_OUTPUT,
                    Prototype<SynapseMessage>::Get()));
    }

    /* variable initialization */
    for(int i=0; i<num_propagators_; i++)
    {
        syn_queue_size_.push_back(0);
        queue_state_.push_back(true);
        synTS_.push_back(false);
    }

    is_empty_ = true;
    coreTS_ = false;
}

void FastSynQueue::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // Process parity message
    SignalMessage *parity_msg = 
        static_cast<SignalMessage*>(inmsgs[PORT_coreTS]);
    if(parity_msg)
    {
        coreTS_ = parity_msg->value;
        INFO_PRINT("[SDQ] Update coreTS parity");
        
        for(int i=0; i<num_propagators_; i++)
            synTS_[i] = coreTS_;
    }

    // Process inputs
    for (int s=0; s<num_propagators_; s++)
    {
        SynapseMessage *syn_msg =
            static_cast<SynapseMessage*> (inmsgs[PORT_syns[s]]);
        SignalMessage *synTS_msg =
            static_cast<SignalMessage*> (inmsgs[PORT_synTS[s]]);

        if(GetOutQueSize(PORT_acc[s]) > 2)
        {
            inmsgs[PORT_syns[s]] = nullptr;
            inmsgs[PORT_synTS[s]] = nullptr;
        }
        else if(syn_msg)
        {
            if(unlikely((synTS_[s] != coreTS_ && 
                            synTS_msg->value != synTS_[s])))
            {
                SIM_ERROR ("Order of synapse data is broken", 
                        GetFullName().c_str());
                return;
            }

            synTS_[s] = synTS_msg->value;
            if(synTS_[s] == coreTS_)
            {
                INFO_PRINT ("[SDQ] Send synapse data (idx: %d)", 
                        syn_msg->idx);
                outmsgs[PORT_acc[s]] = new SynapseMessage (0, 0, 
                        syn_msg->idx);
                queue_state_[s] = false;

                if(is_empty_)
                {
                    outmsgs[PORT_empty] = new IntegerMessage(0);
                    is_empty_ = false;
                }
            }
            else
            {
                INFO_PRINT ("[SDQ] Store in internal queue (core %d, syn %d)",
                        coreTS_, (int)synTS_[s]);
                inmsgs[PORT_syns[s]] = nullptr; 
                inmsgs[PORT_synTS[s]] = nullptr;
                queue_state_[s] = true;
            }
        }
        else
        {
            if(!queue_state_[s] && (GetOutQueSize(PORT_acc[s]) == 0))
            {
                if(outmsgs[PORT_acc[s]] == nullptr)
                    queue_state_[s] = true;       
            }
        }
    }

    if(!is_empty_)
    {
        bool all_empty = true;
        for(int i=0; i<num_propagators_; i++)
        {
            all_empty = all_empty && queue_state_[i];
        }

        if(all_empty)
        {
            delete outmsgs[PORT_empty];
            outmsgs[PORT_empty] = new IntegerMessage(1);
            is_empty_ = true;
        }
    }
}
