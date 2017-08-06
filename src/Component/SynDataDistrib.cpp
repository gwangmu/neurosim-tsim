#include <Component/SynDataDistrib.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/SynapseMessage.h>
#include <Message/SelectMessage.h>
#include <Message/SignalMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

SynDataDistrib::SynDataDistrib (string iname, Component *parent, 
        uint32_t num_propagator)
    : Module ("SynDataDistrib", iname, parent, 1)
{
    SetClock ("dram");
    
    num_prop_ = num_propagator;;
    for (int i=0; i<num_prop_; i++)
    {
        IPORT_Syn.push_back (CreatePort("syn_in" + to_string(i), 
                    Module::PORT_INPUT,
                    Prototype<SynapseMessage>::Get()));
        IPORT_Cidx.push_back (CreatePort("syn_cidx" + to_string(i), 
                    Module::PORT_INPUT,
                    Prototype<SelectMessage>::Get()));
        IPORT_TS.push_back (CreatePort("syn_ts_in" + to_string(i), 
                    Module::PORT_INPUT,
                    Prototype<SignalMessage>::Get()));

        
        OPORT_Syn.push_back (CreatePort("syn_out" + to_string(i), 
                    Module::PORT_OUTPUT,
                    Prototype<SynapseMessage>::Get()));
        OPORT_TS.push_back (CreatePort("syn_ts_out" + to_string(i), 
                    Module::PORT_OUTPUT,
                    Prototype<SignalMessage>::Get()));
    }
}

void SynDataDistrib::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{

    for (int i=0; i<num_prop_; i++)
    {
        SynapseMessage *syn_msg = 
            static_cast<SynapseMessage*>(inmsgs[IPORT_Syn[i]]);
        SelectMessage *sel_msg = 
            static_cast<SelectMessage*> (inmsgs[IPORT_Cidx[i]]);
        SignalMessage *ts_msg =
            static_cast<SignalMessage*> (inmsgs[IPORT_TS[i]]);

        if(syn_msg && sel_msg )
        {
            uint32_t weight = syn_msg->weight;
            uint16_t idx = syn_msg->idx;

            uint8_t rhs = sel_msg->value;

            outmsgs[OPORT_Syn[i]] = new SynapseMessage (rhs, weight, idx);
            outmsgs[OPORT_TS[i]] = new SignalMessage (rhs, ts_msg->value);
            
            INFO_PRINT ("[SDD] Distribute Synapse Data to %d", rhs);
        }
        else if (unlikely (syn_msg || sel_msg))
        {
            SIM_ERROR ("SDD receive only either synapse or select",
                    GetFullName().c_str());
        }
    }
}
