#include <Component/AxonMetaRecv.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

AxonMetaRecv::AxonMetaRecv (string iname, Component *parent)
    : Module ("AxonMetaRecv", iname, parent, 1)
{
    //SetClock ("dram");
    
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    IPORT_Bypass = CreatePort ("axon_bypass", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Axon = CreatePort ("axon_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_delay = CreatePort ("delay_out", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    is_idle_ = false;
}

void AxonMetaRecv::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);
    AxonMessage *bypass_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Bypass]);

    AxonMessage *msg = nullptr;
    if(bypass_msg)
    {
        msg = bypass_msg;
        inmsgs[IPORT_Axon] = nullptr;
    }
    else if(axon_msg)
        msg = axon_msg;

   if(msg)
    {
        INFO_PRINT ("[AMR] Receive message (addr %lu, delay %u)", 
                msg->value, msg->delay);

        if(msg->delay == 0)
            outmsgs[OPORT_Axon] = 
                new AxonMessage (0, msg->value, msg->len, 0, 
                                 msg->target,
                                 msg->is_inh);
        else    
            outmsgs[OPORT_delay] = 
                new AxonMessage (0, msg->value, 
                                msg->len, msg->delay, -1,
                                msg->is_inh);

        if (is_idle_)
        {
            INFO_PRINT ("[AMR] Axon metadata receiver is busy");
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
        }
    }
    else if(!is_idle_)
    {
        INFO_PRINT ("[AMR] Axon metadata receiver is idle");
        is_idle_ = true;
        outmsgs[OPORT_idle] = new IntegerMessage (1);
    }
}
