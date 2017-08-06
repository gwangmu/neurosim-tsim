#include <Component/SynDataQueue.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/IndexMessage.h>
#include <Message/SynapseMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

SynDataQueue::SynDataQueue (string iname, Component *parent, 
        uint32_t max_queue_size)
    : Module ("SynDataQueue", iname, parent, 1)
{
    SetClock ("dram");

    IPORT_Synapse = CreatePort ("syn", Module::PORT_INPUT,
            Prototype<SynapseMessage>::Get());
    IPORT_CoreTS = CreatePort ("core_ts", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    IPORT_SynapseTS = CreatePort ("syn_ts", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());

    OPORT_Acc = CreatePort ("acc", Module::PORT_OUTPUT,
            Prototype<SynapseMessage>::Get());
    OPORT_Empty = CreatePort ("empty", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    is_empty = true;
    this->max_queue_size_ = max_queue_size;
}

void SynDataQueue::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    SynapseMessage *syn_msg = static_cast<SynapseMessage*>(inmsgs[IPORT_Synapse]);
    SignalMessage *coreTS_msg = static_cast<SignalMessage*>(inmsgs[IPORT_CoreTS]);
    SignalMessage *synTS_msg = static_cast<SignalMessage*>(inmsgs[IPORT_SynapseTS]);

    if(/**outque_size*/ GetOutQueSize(OPORT_Acc) > 2)
    {
        inmsgs[IPORT_Synapse] = nullptr;
        inmsgs[IPORT_SynapseTS] = nullptr;
    }
    else if(syn_msg && synTS_msg)
    {
        uint32_t weight = syn_msg->weight;
        uint16_t idx = syn_msg->idx;
       
        INFO_PRINT("[SDQ] Receive synapse data");

        if(unlikely((synTS != coreTS && synTS_msg->value == coreTS)))
        {
            INFO_PRINT ("[SDQ] Fail. coreTS: %d, synTS: %d, msgTS: %d", 
                    coreTS, synTS, synTS_msg->value);
            SIM_ERROR ("Order of synapse data is broken", GetFullName().c_str());
            return;
        }
        
        synTS = synTS_msg->value;
        if(synTS == coreTS)
        {
            INFO_PRINT ("[SDQ] Send synapse data (idx: %d)", idx);
            outmsgs[OPORT_Acc] = new SynapseMessage (0, weight, idx);
    
            if (is_empty)
            {
                is_empty = false;
                INFO_PRINT ("[SDQ] Synapse Data queue has data");

                outmsgs[OPORT_Empty] = new IntegerMessage (0);
            }
        }
        else
        {
            INFO_PRINT ("[SDQ] Store in internal queue (core %d, syn %d)",
                     coreTS, synTS);
            inmsgs[IPORT_Synapse] = nullptr; 
            inmsgs[IPORT_SynapseTS] = nullptr; 
        }

    }
    else if(unlikely(syn_msg || synTS_msg))
    {
        SIM_ERROR ("Synapse Data Queue receive only either weight/idx or parity", GetFullName().c_str());
        return;
    }

    if (coreTS_msg)
    {
        coreTS = coreTS_msg->value;
        synTS = coreTS;
        INFO_PRINT("[SDQ] Update TS parity (%d)", coreTS);
    }

    if(!is_empty && GetOutQueSize(OPORT_Acc) == 0)
    {
        if(outmsgs[OPORT_Acc] == nullptr)
        {
            is_empty = true;
            INFO_PRINT ("[SDQ] Axon metatda queue is empty");

            outmsgs[OPORT_Empty] = new IntegerMessage (1);
        }
    }
}
