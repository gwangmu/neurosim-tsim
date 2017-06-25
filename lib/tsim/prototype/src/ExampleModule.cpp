/***
 * TSim Module prototype (body)
 **/

#include <Prototype/ExampleModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleModule::ExampleModule (string iname, Component *parent)
{
    #if 0
    // (TODO) create ports
    PORT_SPIKEIN = CreatePort ("spikein", Port::INPUT, new SpikeMessage ());
    PORT_SPIKEOUT = CreatePort ("spikeout", Port::OUTPUT, new SpikeMessage ());

    // (OPTIONAL) initialize fields
    counter = 0
    #endif
}

// NOTE: called only if not stalled
void ExampleModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    #if 0
    // (TODO) receive incoming messages/instruction, 
    //  emulate operation (or delay), emit messages.

    DataMessage *datamsg = static_cast<DataMessage *>(inmsgs[PORT_DATAIN]);

    if (datamsg)
    {
        outmsgs[PORT_SPIKEOUT] = new SpikeMessage (counter);
        counter++;
    }
    #endif
}
