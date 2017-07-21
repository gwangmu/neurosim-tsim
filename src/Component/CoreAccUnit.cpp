#include <Component/CoreAccUnit.h>

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

// SynDataQueue(1) -> Acc -> DG read -> Accum --> DG write
const uint16_t acc_depth = 3; 

CoreAccUnit::CoreAccUnit (string iname, Component *parent,
        uint32_t num_propagators)
    : Module ("CoreAccUnitModule", iname, parent, 1)
{
    PORT_coreTS = CreatePort ("coreTS", Module::PORT_INPUT, 
            Prototype<SignalMessage>::Get());
    PORT_accfin = CreatePort ("accfin", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());
    
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
    this->pipeline_depth_ = acc_depth;
    acc_state_ = 0;
    acc_mask_ = (1 << (pipeline_depth_)) - 2;

    sent_accfin_ = true;
    syn_rr_ = 0;
    
    synTS_ = false;
    coreTS_ = false;
}

void CoreAccUnit::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    // Process parity message
    SignalMessage *parity_msg = 
        static_cast<SignalMessage*>(inmsgs[PORT_coreTS]);
    if(parity_msg)
    {
        coreTS_ = parity_msg->value;
        synTS_ = coreTS_;
        INFO_PRINT("[Acc] Update coreTS parity");
    }

    // Process inputs
    SynapseMessage *syn_msg;
    SignalMessage *synTS_msg, *ts_msg;
    bool queue_empty = true;
   
    INFO_PRINT ("[Acc] Operation (rr: %d) %p %p", 
            syn_rr_, inmsgs[PORT_synTS[0]], inmsgs[PORT_synTS[1]]);
    for(int i=0; i<num_propagators_; i++)
    {
        ts_msg = static_cast<SignalMessage*>(inmsgs[PORT_synTS[i]]);
        if(ts_msg)
        {
            queue_empty = queue_empty &&
                (ts_msg->value != coreTS_);
        }

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
    
    acc_state_ = (acc_state_ << 1) & acc_mask_;
    if(syn_msg)
    {
        INFO_PRINT("[Acc] Receive synapse data (rr: %d)", syn_rr_);
        if(unlikely((synTS_ != coreTS_ && synTS_msg->value != synTS_)))
        {
            INFO_PRINT ("[Acc] Fail. coreTS: %d, synTS: %d, msgTS: %d", 
                    coreTS_, synTS_, synTS_msg->value);
            SIM_ERROR ("Order of synapse data is broken", 
                    GetFullName().c_str());
            return;
        }
   
        synTS_ = synTS_msg->value;
        if(synTS_ == coreTS_)
        {
            INFO_PRINT ("[Acc] Request deltaG (idx: %d)", syn_msg->idx);
            if(sent_accfin_)
            {
                INFO_PRINT ("[Acc] Acc Unit is busy");
                outmsgs[PORT_accfin] = new SignalMessage (0, false);
                sent_accfin_ = false;
            }

            acc_state_ |= 0x1;
        }
        else
        {
            INFO_PRINT ("[Acc] Store in internal queue (core %d, syn %d)",
                     coreTS_, synTS_);
            inmsgs[PORT_syns[syn_rr_]] = nullptr; 
            inmsgs[PORT_synTS[syn_rr_]] = nullptr; 
        }
    }
    else if(queue_empty && (!acc_state_))
    {
        if(!sent_accfin_)
        {
            INFO_PRINT ("[Acc] Acc Unit is idle");
            outmsgs[PORT_accfin] = new SignalMessage (0, true);
            sent_accfin_ = true;
        }
    }

    syn_rr_ = (syn_rr_ + 1) % num_propagators_;  
}





