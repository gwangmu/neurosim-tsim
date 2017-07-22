#include <Component/AxonClassifier.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/DramMessage.h>
#include <Message/IndexMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SynapseMessage.h>
#include <Message/AxonMessage.h>
#include <Message/SelectMessage.h>
#include <Message/DelayMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonClassifier::AxonClassifier (string iname, Component *parent)
    : Module ("AxonClassifier", iname, parent, 1)
{
    SetClock ("dram");
    
    IPORT_Dram = CreatePort ("dram", Module::PORT_INPUT,
            Prototype<DramMessage>::Get());
    IPORT_TSparity = CreatePort ("ts_parity", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());

    OPORT_Synapse = CreatePort ("syn_out", Module::PORT_OUTPUT,
            Prototype<SynapseMessage>::Get());
    OPORT_TSparity = CreatePort ("ts_out", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    
    OPORT_Axon = CreatePort ("axon_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Delay = CreatePort ("delay_out", Module::PORT_OUTPUT,
            Prototype<DelayMessage>::Get());
    OPORT_BoardID = CreatePort ("board_id", Module::PORT_OUTPUT,
            Prototype<SelectMessage>::Get());
    OPORT_Sel = CreatePort ("tar_idx", Module::PORT_OUTPUT,
            Prototype<SelectMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    is_idle_ = false;
}

void AxonClassifier::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    DramMessage *dram_msg = static_cast<DramMessage*>(inmsgs[IPORT_Dram]);

    if(dram_msg)
    {
        bool is_board = dram_msg->intra_board;

        if(is_board)
        {
            outmsgs[OPORT_Axon] = new AxonMessage (0, dram_msg->val32, dram_msg->val16);
            outmsgs[OPORT_BoardID] = new SelectMessage (0, dram_msg->target_idx); 
            INFO_PRINT ("[AEC] Send routing information to controller");
        }
        else
        {
            uint32_t chip_idx = dram_msg->dest_idx;
            outmsgs[OPORT_Synapse] = new SynapseMessage (chip_idx, dram_msg->val32, dram_msg->val16);
            outmsgs[OPORT_TSparity] = new SignalMessage (chip_idx, ts_parity);
            outmsgs[OPORT_Sel] = new SelectMessage (chip_idx, dram_msg->target_idx); 
            INFO_PRINT ("[AEC] Send synapse data to chip %u (idx %u)"
                    , chip_idx, dram_msg->val16);
        }

        if (is_idle_)
        {
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
            INFO_PRINT ("[AEC] Axon entry classifier is busy");
        }
    }
    else if(*outque_size == 0 && !is_idle_)
    {
        is_idle_ = true;
        outmsgs[OPORT_idle] = new IntegerMessage (1);
        INFO_PRINT ("[AEC] Axon entry classifier is idle");
    }

    IntegerMessage *parity_msg = static_cast<IntegerMessage*>(inmsgs[IPORT_TSparity]);
    if(parity_msg)
    {
        if(parity_msg->value != ts_parity)
        {
            ts_parity = parity_msg->value;
            INFO_PRINT ("[AEC] Update TS parity");
        }
    }


}
