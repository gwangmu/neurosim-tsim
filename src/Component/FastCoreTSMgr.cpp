#include <Component/FastCoreTSMgr.h>

#include <Message/SignalMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

FastCoreTSMgr::FastCoreTSMgr (string iname, Component* parent)
    : Module ("FastCoreTSMgr", iname, parent, 1)
{
    PORT_DynEnd = CreatePort ("dyn_end", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_Acc = CreatePort ("Acc_idle", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_SDQ = CreatePort ("SDQ_empty", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    PORT_curTS = CreatePort ("curTS", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());

    PORT_TSparity = CreatePort ("Tsparity", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    PORT_DynFin = CreatePort ("DynFin", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    for (int i=0; i<3; i++)
        state[i] = true;

    cur_tsparity_ = -1;
    next_tsparity_ = -1;
    is_dynfin = true;
}

void FastCoreTSMgr::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /************* Check core components  ****************/
    SignalMessage *end_msg = static_cast<SignalMessage*>(inmsgs[PORT_DynEnd]);
    SignalMessage *acc_msg = static_cast<SignalMessage*>(inmsgs[PORT_Acc]);
    IntegerMessage *sdq_msg = static_cast<IntegerMessage*>(inmsgs[PORT_SDQ]);

    INFO_PRINT ("[CoTS] Operation %p", end_msg);

    if(end_msg)
    {
        INFO_PRINT ("[CoTS] CoreDynUnit is end? %d", end_msg->value);
        
        /*** Check state of dynamics modules  ***/
        bool dynfin = end_msg->value;
        if(!is_dynfin && dynfin)
        {
            INFO_PRINT ("[CoTS] Dynamics finished");
            outmsgs[PORT_DynFin] = new IntegerMessage (1);
            is_dynfin = true;
            state[DYN] = true;
        }
        else if (is_dynfin && !dynfin)
        {
            outmsgs[PORT_DynFin] = new IntegerMessage (0);
            is_dynfin = false;
            state[DYN] = false;
        }
    
    }
    if(acc_msg)
    {
        INFO_PRINT ("[CoTS] Accumulator is end? %d", acc_msg->value);
        state[Acc] = acc_msg->value;
    }
    if(sdq_msg)
    {
        if(state[SDQ] != sdq_msg->value)
            INFO_PRINT ("[CoTS] Synapse data queue is end? %lu", sdq_msg->value);
        state[SDQ] = sdq_msg->value;
    }

    /*** Update TS parity ***/
    IntegerMessage *parity_msg = static_cast<IntegerMessage*>(inmsgs[PORT_curTS]);
    if(parity_msg)
    {
        if(next_tsparity_ != parity_msg->value) 
            INFO_PRINT ("[CoTS] Update TS parity to %lu", parity_msg->value);
        next_tsparity_ = parity_msg->value;
    }

    if(cur_tsparity_ != next_tsparity_)
    {
        bool all_finish = true;
        for (int i=0; i<3; i++)
            all_finish = all_finish && state[i];

        if (all_finish)
        {
            cur_tsparity_ = next_tsparity_;
            outmsgs[PORT_TSparity] = new SignalMessage (-1, cur_tsparity_);

            // Initiate Neuron Block Controller, change its state
            INFO_PRINT ("[CoTS] Update Core TS parity to %d", cur_tsparity_);
            state[DYN] = false;
        }
    }
}


