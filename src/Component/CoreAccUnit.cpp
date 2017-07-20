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
const uint16_t acc_depth = 4; 

CoreAccUnit::CoreAccUnit (string iname, Component *parent)
    : Module ("CoreAccUnitModule", iname, parent, acc_depth)
{
    PORT_syn = CreatePort ("syn", Module::PORT_INPUT, 
            Prototype<SynapseMessage>::Get());
    PORT_accfin = CreatePort ("accfin", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());
    
    /* variable initialization */
    this->pipeline_depth_ = acc_depth;


    sent_accfin_ = true;
}

void CoreAccUnit::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    // Process inputs
    SynapseMessage *syn_msg = static_cast<SynapseMessage*>(inmsgs[PORT_syn]);;
    if(syn_msg)
    {
        INFO_PRINT("[Acc] Receive synapse data");
        if(sent_accfin_)
        {
            outmsgs[PORT_accfin] = new SignalMessage (0, false);
            sent_accfin_ = false;
        }
    }
    else
    {
        if(!sent_accfin_)
        {
            outmsgs[PORT_accfin] = new SignalMessage (0, true);
            sent_accfin_ = true;
        }
    }
}





