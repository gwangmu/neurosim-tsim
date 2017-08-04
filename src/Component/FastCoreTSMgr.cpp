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
    PORT_AccIdle = CreatePort ("acc_idle", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_SynEmpty = CreatePort ("syn_empty", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    PORT_curTS = CreatePort ("curTS", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());

    PORT_TSparity = CreatePort ("Tsparity", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    PORT_DynFin = CreatePort ("DynFin", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    dyn_end_ = true;
    acc_idle_ = true;
    syn_empty_ = true;

    cur_tsparity_ = -1;
    next_tsparity_ = -1;
}

void FastCoreTSMgr::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /************* Check core components  ****************/
    SignalMessage *end_msg = static_cast<SignalMessage*>(inmsgs[PORT_DynEnd]);
    SignalMessage *idle_msg = 
        static_cast<SignalMessage*>(inmsgs[PORT_AccIdle]);
    IntegerMessage *empty_msg = 
        static_cast<IntegerMessage*>(inmsgs[PORT_SynEmpty]);

    if(end_msg)
    {
        //INFO_PRINT ("[CoTS] CoreDynUnit is end? %d", end_msg->value);
        
        /*** Check state of dynamics modules  ***/
        bool dynfin = end_msg->value;
        if(!dyn_end_ && dynfin)
        {
            INFO_PRINT ("[CoTS] Dynamics finished");
            outmsgs[PORT_DynFin] = new IntegerMessage (1);
            dyn_end_ = true;
        }
        else if (dyn_end_ && !dynfin)
        {
            INFO_PRINT ("[CoTS] Dynamics did not finish");
            outmsgs[PORT_DynFin] = new IntegerMessage (0);
            dyn_end_ = false;
        }
    
    }
    if(idle_msg)
    {
        if(idle_msg->value != acc_idle_)
            INFO_PRINT ("[CoTS] Accumulator is end? %d", idle_msg->value);
        acc_idle_ = idle_msg->value;
    }
    if(empty_msg)
    {
        if(empty_msg->value != syn_empty_)
            INFO_PRINT ("[CoTS] Synapse data queue is empty %d", empty_msg->value);
        syn_empty_ = empty_msg->value;
    }

    /*** Update TS parity ***/
    IntegerMessage *parity_msg = static_cast<IntegerMessage*>(inmsgs[PORT_curTS]);
    if(parity_msg)
    {
        if(next_tsparity_ != parity_msg->value) 
            INFO_PRINT ("[CoTS](%s) Update TS parity to %lu", 
                    GetParent()->GetName().c_str(), parity_msg->value);
        next_tsparity_ = parity_msg->value;
    }

    if(cur_tsparity_ != next_tsparity_)
    {
        bool all_finish = dyn_end_ && acc_idle_ && syn_empty_;

        if (all_finish)
        {
            cur_tsparity_ = next_tsparity_;
            outmsgs[PORT_TSparity] = new SignalMessage (-1, cur_tsparity_);

            // Initiate Neuron Block Controller, change its state
            INFO_PRINT ("[CoTS](%s) Update Core TS parity to %d", 
                    GetParent()->GetName().c_str(), cur_tsparity_);
            dyn_end_ = false;

            delete outmsgs[PORT_DynFin];
            outmsgs[PORT_DynFin] = new IntegerMessage (0);
        }
    }
}


