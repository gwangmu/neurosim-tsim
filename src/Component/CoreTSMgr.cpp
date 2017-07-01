#include <Component/CoreTSMgr.h>

#include <Message/SignalMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

CoreTSMgr::CoreTSMgr (string iname, Component* parent)
    : Module ("CoreTSMgr", iname, parent, 1)
{
    PORT_NBC = CreatePort ("NBC_end", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_NB = CreatePort ("NB_idle", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_AMQ = CreatePort ("AMQ_empty", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_Acc = CreatePort ("Acc_idle", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_SDQ = CreatePort ("SDQ_empty", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    PORT_curTS = CreatePort ("curTS", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());

    PORT_TSparity = CreatePort ("Tsparity", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    PORT_DynFin = CreatePort ("DynFin", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    for (int i=0; i<5; i++)
        state[i] = true;

    cur_tsparity_ = -1;
    next_tsparity_ = -1;
}

void CoreTSMgr::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    /************* Check core components  ****************/
    SignalMessage *nbc_msg = static_cast<SignalMessage*>(inmsgs[PORT_NBC]);
    SignalMessage *nb_msg = static_cast<SignalMessage*>(inmsgs[PORT_NB]);
    SignalMessage *amq_msg = static_cast<SignalMessage*>(inmsgs[PORT_AMQ]);
    SignalMessage *acc_msg = static_cast<SignalMessage*>(inmsgs[PORT_Acc]);
    SignalMessage *sdq_msg = static_cast<SignalMessage*>(inmsgs[PORT_SDQ]);

    if(nbc_msg)
    {
        DEBUG_PRINT ("[TSMgr] NB Controller is end? %d", nbc_msg->value);
        state[NBC] = nbc_msg->value;
    }
    if(nb_msg)
    {
        DEBUG_PRINT ("[TSMgr] NB Controller is end? %d", nb_msg->value);
        state[NB] = nb_msg->value;
    }
    if(amq_msg)
    {
        DEBUG_PRINT ("[TSMgr] NB Controller is end? %d", amq_msg->value);
        state[AMQ] = amq_msg->value;
    }
    if(acc_msg)
    {
        DEBUG_PRINT ("[TSMgr] NB Controller is end? %d", acc_msg->value);
        state[Acc] = acc_msg->value;
    }
    if(sdq_msg)
    {
        DEBUG_PRINT ("[TSMgr] NB Controller is end? %d", sdq_msg->value);
        state[SDQ] = sdq_msg->value;
    }


    /*** Check state of dynamics modules  ***/
    bool dynfin = state[NBC] && state[NB] && state[AMQ];
    if(dynfin)
    {
        DEBUG_PRINT ("[TSMgr] Dynamics finished");
        outmsgs[PORT_DynFin] = new SignalMessage (0, true);
    }

    /*** Update TS parity ***/
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[PORT_curTS]);
    if(parity_msg)
    {
        next_tsparity_ = parity_msg->value;
        DEBUG_PRINT ("[TSMgr] Update TS parity to %d", next_tsparity_);
    }

    if(cur_tsparity_ != next_tsparity_)
    {
        bool all_finish = true;
        for (int i=0; i<5; i++)
            all_finish = all_finish && state[i];

        if (all_finish)
        {
            cur_tsparity_ = next_tsparity_;
            outmsgs[PORT_TSparity] = new SignalMessage (0, cur_tsparity_);

            // Initiate Neuron Block Controller, change its state
            state[NBC] = false;
        }
    }
}


