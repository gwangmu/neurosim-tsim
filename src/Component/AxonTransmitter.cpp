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
    idle_delay_ = 0;
}

void AxonTransmitter::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg)
    {
        INFO_PRINT ("[AT] Receive axon message (addr %lu)", axon_msg->value);

        uint8_t prop_idx = axon_msg->value / dram_size;
        uint64_t addr = axon_msg->value % dram_size;
        outmsgs[OPORT_Axons[prop_idx]] = 
            new AxonMessage (0, addr, axon_msg->len, axon_msg->delay); 
        
        if (is_idle_)
        {
            INFO_PRINT ("[AT] Axon metadata transmitter is busy");
            is_idle_ = false;
            outmsgs[OPORT_idle] = new IntegerMessage (0);
        }
        idle_delay_ = 3; // FIXME Delay between chip and propagator
    }
    else if(!is_idle_ && /**outque_size == 0*/ GetOutQueSize(OPORT_idle) == 0)
    {
        if(idle_delay_ == 0)
        {
            INFO_PRINT ("[AT] Axon metadata transmitter is idle");
            is_idle_ = true;
            outmsgs[OPORT_idle] = new IntegerMessage (1);
        }
        idle_delay_--;
    }
}
