#include <Component/AxonTransmitter.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Simulation/Testbench.h>

#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

AxonTransmitter::AxonTransmitter (string iname, Component *parent) 
    : Module ("AxonTransmitter", iname, parent, 1)
{
    this->num_propagator = GET_PARAMETER(num_propagators);
    this->dram_size = GET_PARAMETER(dram_size);
    
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    for (int i=0; i<num_propagator; i++)
    {
        OPORT_Axons.push_back( 
                CreatePort ("axon_out" + to_string(i), Module::PORT_OUTPUT, 
                    Prototype<AxonMessage>::Get()));
    }

    is_idle_ = false;

}

void AxonTransmitter::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg)
    {
        DEBUG_PRINT ("[AT] Receive axon message (addr %lu)", axon_msg->value);

        uint8_t prop_idx = axon_msg->value / dram_size;
        outmsgs[OPORT_Axons[prop_idx]] = new AxonMessage (0, axon_msg->value, axon_msg->len); 
        
        if (is_idle_)
        {
            DEBUG_PRINT ("[AT] Axon metadata transmitter is busy");
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
        }
    }
    else if(!is_idle_ && *outque_size==0)
    {
        DEBUG_PRINT ("[AT] Axon metadata transmitter is idle");
        is_idle_ = true;
        outmsgs[OPORT_idle] = new IntegerMessage (1);
    }
}
