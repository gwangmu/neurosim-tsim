#include <Component/CoreAccUnit.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Register/EmptyRegister.h>
#include <Register/EmptyRegisterWord.h>
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

CoreAccUnit::CoreAccUnit (string iname, Component *parent)
    : Module ("CoreAccUnitModule", iname, parent, 1)
{
    PORT_syn = CreatePort ("syn", Module::PORT_INPUT, 
            Prototype<SynapseMessage>::Get());
    PORT_accfin = CreatePort ("accfin", Module::PORT_OUTPUT, 
            Prototype<SignalMessage>::Get());
    
    /* variable initialization */
    this->pipeline_depth_ = acc_depth;
    acc_state_ = 0;
    acc_mask_ = (1 << (pipeline_depth_)) - 2;


    sent_accfin_ = true;

    Register::Attr regattr (10, 10);
    SetRegister (new EmptyRegister (Register::SRAM, regattr));
}

void CoreAccUnit::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // Process inputs
    SynapseMessage *syn_msg = static_cast<SynapseMessage*>(inmsgs[PORT_syn]);;
    if(syn_msg)
    {
        INFO_PRINT("[Acc] Request deltaG (idx: %d)",
                syn_msg->idx);
        if(sent_accfin_)
        {
            outmsgs[PORT_accfin] = new SignalMessage (0, false);
            sent_accfin_ = false;
        }

        acc_state_ |= 1;
            
        GetRegister()->GetWord (0);
        GetRegister()->SetWord (0, 0);

    }
    else
    {
        if(!sent_accfin_ && (acc_state_ == 0))
        {
            outmsgs[PORT_accfin] = new SignalMessage (0, true);
            sent_accfin_ = true;
        }
    }
    
    // Update state
    acc_state_ = (acc_state_ << 1) & acc_mask_;
    
}





