#include <Component/Accumulator.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/IndexMessage.h>
#include <Message/SynapseMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/SignalMessage.h>

#include <Script/SpikeFileScript.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

Accumulator::Accumulator (string iname, Component *parent)
    : Module ("AccumulatorModule", iname, parent, 1)
{
    PORT_in = CreatePort ("synapse", Module::PORT_INPUT,
            Prototype<SynapseMessage>::Get());
    PORT_TSparity = CreatePort ("tsparity", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    
    PORT_raddr = CreatePort ("r_addr", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    PORT_rdata = CreatePort ("r_data", Module::PORT_INPUT,
            Prototype<DeltaGMessage>::Get());
    PORT_waddr = CreatePort ("w_addr", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    PORT_wdata = CreatePort ("w_data", Module::PORT_OUTPUT,
            Prototype<DeltaGMessage>::Get());
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    /* variable initialization */
    is_idle_ = true;
    ts_parity = false;
}

void Accumulator::Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr)
{
    SynapseMessage *syn_msg = static_cast<SynapseMessage*>(inmsgs[PORT_in]);
    if(syn_msg)
    {
        outmsgs[PORT_raddr] = new IndexMessage (0, syn_msg->idx);
        idx_queue.push_back(syn_msg->idx);

        DEBUG_PRINT ("[Acc] Request deltaG (idx: %d)", syn_msg->idx);

        if(is_idle_)
        {
            is_idle_ = false;
            outmsgs[PORT_idle] = new SignalMessage (0, false);
        }
    }

    DeltaGMessage *dg_msg = static_cast<DeltaGMessage*>(inmsgs[PORT_rdata]);
    if(dg_msg)
    {
        int idx = idx_queue.front();
        idx_queue.pop_front();

        // Accumulation
        DEBUG_PRINT ("[Acc] Accumulation (idx: %d)", idx);
        
        outmsgs[PORT_waddr] = new IndexMessage (0, idx);
        outmsgs[PORT_wdata] = new DeltaGMessage (0, 0);
    }
    else if (!is_idle_ && idx_queue.empty())
    {
        is_idle_ = true;
        DEBUG_PRINT ("[Acc] Accumulator is idle");

        outmsgs[PORT_idle] = new SignalMessage (0, true); 
    }

    // Update TS parity
    SignalMessage *parity_msg = static_cast<SignalMessage*>(inmsgs[PORT_TSparity]);
    if (parity_msg)
    {
        ts_parity = parity_msg->value;
        DEBUG_PRINT ("[Acc] Update TS parity (%d)", ts_parity);
    }  

}









