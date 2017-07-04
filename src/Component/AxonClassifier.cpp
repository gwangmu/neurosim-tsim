#include <Component/AxonClassifier.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/DramMessage.h>
#include <Message/IndexMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SynapseMessage.h>
#include <Message/AxonMessage.h>
#include <Message/SelectMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonClassifier::AxonClassifier (string iname, Component *parent)
    : Module ("AxonClassifier", iname, parent, 1)
{
    IPORT_Dram = CreatePort ("dram", Module::PORT_INPUT,
            Prototype<DramMessage>::Get());
    IPORT_TSparity = CreatePort ("ts_parity", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());

    OPORT_Synapse = CreatePort ("syn_out", Module::PORT_OUTPUT,
            Prototype<SynapseMessage>::Get());
    OPORT_TSparity = CreatePort ("ts_out", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    
    OPORT_Axon = CreatePort ("axon_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Sel = CreatePort ("tar_idx", Module::PORT_OUTPUT,
            Prototype<SelectMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    is_idle_ = true;
}

void AxonClassifier::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    DramMessage *dram_msg = static_cast<DramMessage*>(inmsgs[IPORT_Dram]);
    if(dram_msg)
    {
        bool is_board = dram_msg->intra_board;
        int target_module = is_board? 1:0; // 0: chip, 1: controller

        outmsgs[OPORT_Sel] = new SelectMessage (target_module, dram_msg->target_idx); 
        if(is_board)
        {
            outmsgs[OPORT_Axon] = new AxonMessage (0, dram_msg->val32, dram_msg->val16);
            DEBUG_PRINT ("[AEC] Send routing information to controller");
        }
        else
        {
            outmsgs[OPORT_Synapse] = new SynapseMessage (0, dram_msg->val32, dram_msg->val16);
            outmsgs[OPORT_TSparity] = new SignalMessage (0, ts_parity);
            DEBUG_PRINT ("[AEC] Send synapse data to chip");
        }
    }

    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[IPORT_TSparity]);
    if(parity_msg)
    {
        ts_parity = parity_msg->value;
        DEBUG_PRINT ("[AEC] Update TS parity");
    }


    if(*outque_size == 0 && !is_idle_)
    {
        is_idle_ = true;
        outmsgs[OPORT_idle] = new SignalMessage (0, is_idle_);
    }
    else if (*outque_size != 0 && is_idle_)
    {
        is_idle_ = false;
        outmsgs[OPORT_idle] = new SignalMessage (0, is_idle_);
    }
}
