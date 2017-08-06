#include <Component/NBController.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

NBController::NBController (string iname, Component *parent, uint32_t max_index) 
    : Module ("NBController", iname, parent, 1)
{
    IPORT_DeltaG = CreatePort ("deltaG", Module::PORT_INPUT, 
            Prototype<DeltaGMessage>::Get());
    IPORT_State = CreatePort ("state", Module::PORT_INPUT, 
            Prototype<StateMessage>::Get());
    IPORT_Tsparity = CreatePort ("tsparity", Module::PORT_INPUT, 
            Prototype<SignalMessage>::Get());
    
    OPORT_End = CreatePort ("end", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());
    OPORT_sSRAM = CreatePort ("ssram", Module::PORT_OUTPUT, 
            Prototype<IndexMessage>::Get());
    OPORT_dSRAM = CreatePort ("dsram", Module::PORT_OUTPUT, 
            Prototype<IndexMessage>::Get());
    OPORT_NB = CreatePort ("neuron_block", Module::PORT_OUTPUT, 
            Prototype<NeuronBlockInMessage>::Get());

    /* variable initialization */
    max_idx_ = max_index;
    read_idx_counter_ = 0;
    nb_idx_counter_ = 0;
    ts_parity_ = false;
    is_finish_ = true;
}

void NBController::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    /* Process Inputs */
    StateMessage *state_msg = static_cast<StateMessage*>(inmsgs[IPORT_State]);
    DeltaGMessage *deltaG_msg = static_cast<DeltaGMessage*>(inmsgs[IPORT_DeltaG]);
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[IPORT_Tsparity]);

    if(parity_msg)
    {
        INFO_PRINT ("[NBC] Receive TSParity. Reset NB controller");
        ts_parity_ = parity_msg->value;
        read_idx_counter_ = 0;
        nb_idx_counter_ = 0;
        
        if(is_finish_)
        {
            INFO_PRINT("[NBC] Start NB controller");
            outmsgs[OPORT_End] = new SignalMessage(0, false);
            is_finish_ = false;
        }
    }

    if(state_msg && deltaG_msg)
    {
        INFO_PRINT ("[NBC] Receive state/delta message from SRAM");
        state_reg = state_msg->value;
        deltaG_reg = deltaG_msg->value;
        
        INFO_PRINT ("[NBC] Initiate neuron block (Nidx: %d)", nb_idx_counter_);
        outmsgs[OPORT_NB] = new NeuronBlockInMessage (0, nb_idx_counter_, state_reg, deltaG_reg);

        nb_idx_counter_++;


        if (nb_idx_counter_ == max_idx_)
        {
            INFO_PRINT ("[NBC] Finish controller jobs");
            outmsgs[OPORT_End] = new SignalMessage(0, true);
            is_finish_ = true;
        }
    }
    else if (unlikely((state_msg || deltaG_msg)))
    {
        SIM_ERROR ("State SRAM and delta SRAM are asynched", GetFullName().c_str()); 
        return;
    }

    if(read_idx_counter_ != max_idx_ && !is_finish_)
    {
        INFO_PRINT ("[NBC] Read Reqest");
        outmsgs[OPORT_sSRAM] = new IndexMessage (0, read_idx_counter_);
        outmsgs[OPORT_dSRAM] = new IndexMessage (0, read_idx_counter_);
        read_idx_counter_++;
    }
}
