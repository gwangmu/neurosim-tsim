#include <Component/FastSynQueue.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

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
            Prototype<SignalMessage>::Get());
    PORT_acc = CreatePort ("acc", Module::PORT_OUTPUT,
            Prototype<SynapseMessage>::Get());

    this->num_propagators_ = num_propagators;
    for(int i=0; i<num_propagators_; i++)
    {
        PORT_syns.push_back(
                CreatePort ("syn" + to_string(i), Module::PORT_INPUT,
                    Prototype<SynapseMessage>::Get()));
        
        PORT_synTS.push_back(
                CreatePort ("synTS" + to_string(i), Module::PORT_INPUT,
                    Prototype<SignalMessage>::Get()));
    }

    /* variable initialization */
    for(int i=0; i<num_propagators_; i++)
    {
        syn_queue_size_.push_back(0);
    }

    is_empty_ = true;

    syn_rr_ = 0;
    
    synTS_ = false;
    coreTS_ = false;
}

void FastSynQueue::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    // Process parity message
    SignalMessage *parity_msg = 
        static_cast<SignalMessage*>(inmsgs[PORT_coreTS]);
    if(parity_msg)
    {
        coreTS_ = parity_msg->value;
        synTS_ = coreTS_;
        INFO_PRINT("[FSQ] Update coreTS parity");
    }

    // Process inputs
    SynapseMessage *syn_msg;
    SignalMessage *synTS_msg;
    bool queue_empty = *outque_size == 0;
    for(int i=0; i<num_propagators_; i++)
    {
        if(i==syn_rr_)
        {
            syn_msg = static_cast<SynapseMessage*>(inmsgs[PORT_syns[i]]);
            synTS_msg = static_cast<SignalMessage*>(inmsgs[PORT_synTS[i]]);
        }
        else
        {
            inmsgs[PORT_syns[i]] = nullptr;
            inmsgs[PORT_synTS[i]] = nullptr;
        }
    }

    if(syn_msg)
    {
        INFO_PRINT("[FSQ] Receive synapse data");
        if(unlikely((synTS_ != coreTS_ && synTS_msg->value != synTS_)))
        {
            INFO_PRINT ("[FSQ] Fail. coreTS: %d, synTS: %d, msgTS: %d", 
                    coreTS_, synTS_, synTS_msg->value);
            SIM_ERROR ("Order of synapse data is broken", 
                    GetFullName().c_str());
            return;
        }
   

        synTS_ = synTS_msg->value;
        if(synTS_ == coreTS_)
        {
            INFO_PRINT ("[FSQ] Send synapse data (idx: %d)", 0);
            outmsgs[PORT_acc] = new SynapseMessage (0, 0, 0);
    
            if(is_empty_)
            {
                outmsgs[PORT_empty] = new SignalMessage (0, false);
                is_empty_ = false;
            }
        }
        else
        {
            INFO_PRINT ("[FSQ] Store in internal queue (core %d, syn %d)",
                     coreTS_, synTS_);
            inmsgs[PORT_syns[syn_rr_]] = nullptr; 
            inmsgs[PORT_synTS[syn_rr_]] = nullptr; 
        }
    }
    else if(queue_empty)
    {
        if(!is_empty_)
        {
            outmsgs[PORT_empty] = new SignalMessage (0, true);
            is_empty_ = true;
        }
    }
}





