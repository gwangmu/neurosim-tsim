/***
 * TSim Module prototype (body)
 **/

#include <TSim/Prototype/ExampleModule.h>
#include <TSim/Utility/Prototype.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleModule::ExampleModule (string iname, Component *parent)
{
    #if 0
    // (TODO) create ports
    PORT_SPIKEIN = CreatePort ("spikein", Module::PORT_INPUT, Prototype<ExampleMessage>::Get());
    PORT_SPIKEOUT = CreatePort ("spikeout", Module::PORT_OUTPUT, Prototype<ExampleMessage>::Get());

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

    ExampleMessage *inmsg = static_cast<ExampleMessage *>(inmsgs[PORT_SPIKEIN]);

    if (inmsg)
    {
        outmsgs[PORT_SPIKEOUT] = new ExampleMessage (0, counter);
        counter++;
    }
    #endif
}
