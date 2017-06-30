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
    OPORT_SRAM = CreatePort ("sram", Module::PORT_OUTPUT, 
            Prototype<IndexMessage>::Get());
    OPORT_NB = CreatePort ("neuron_block", Module::PORT_OUTPUT, 
            Prototype<NeuronBlockInMessage>::Get());

    /* variable initialization */
    max_idx_ = max_index;
    idx_counter_ = 0;
    ts_parity_ = false;
}

void NBController::Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr)
{
    /* Process Inputs */
    StateMessage *state_msg = static_cast<StateMessage*>(inmsgs[IPORT_State]);
    DeltaGMessage *deltaG_msg = static_cast<DeltaGMessage*>(inmsgs[IPORT_DeltaG]);
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[IPORT_Tsparity]);

    if(parity_msg)
    {
        DEBUG_PRINT ("[NBC] Receive TSParity. Reset NB controller");
        ts_parity_ = parity_msg->value;
        idx_counter_ = 0;
    }

    if(state_msg && deltaG_msg)
    {
        DEBUG_PRINT ("[NBC] Receive state/delta message from SRAM");
        state_reg = state_msg->value;
        deltaG_reg = deltaG_msg->value;
        
        DEBUG_PRINT ("[NBC] Initiate neuron block (Nidx: %d)", idx_counter_);
        outmsgs[OPORT_NB] = new NeuronBlockInMessage (0, idx_counter_, state_reg, deltaG_reg);
    }
    else if (unlikely((state_msg || deltaG_msg)))
    {
        SIM_ERROR ("State SRAM and delta SRAM are asynched", GetFullName().c_str()); 
        return;
    }

    if(idx_counter_ != max_idx_)
    {
        DEBUG_PRINT ("[NBC] Read Reqest");
        outmsgs[OPORT_SRAM] = new IndexMessage (-1, idx_counter_);
        idx_counter_++;
    }
    else
    {
        DEBUG_PRINT ("[NBC] Finish controller jobs");
        outmsgs[OPORT_End] = new SignalMessage(0, true);
    }
}
