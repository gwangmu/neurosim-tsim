#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataSourceModule.h>
#include <Script/ExampleFileScript.h>
#include <Script/ExampleInstruction.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSourceModule::DataSourceModule (string iname, Component *parent)
    : Module ("DataSourceModule", iname, parent, 1)
{
    // create ports
    PORT_DATAOUT = CreatePort ("dataout",
            Module::PORT_OUTPUT, Prototype<SignalMessage>::Get());

    // init script
    SetScript (new ExampleFileScript ());

    counter = 0;
    is_idle = true;
}

// NOTE: called only if not stalled
void DataSourceModule::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    ExampleInstruction *ininstr = static_cast<ExampleInstruction *>(instr);

    uint32_t instrval = (ininstr ? ininstr->data3 : 0);

    if (ininstr)
        DEBUG_PRINT ("instruction received (%s, %s, %u)",
                ininstr->data1.c_str(), ininstr->data2.c_str(), ininstr->data3);

    //State s;
    //counter++;
    //outmsgs[PORT_DATAOUT] = new SignalMessage (0, counter, s, 0);
    
    if (is_idle)
    {
        outmsgs[PORT_DATAOUT] = new SignalMessage (0, false);
        is_idle = false;
    }

    // If condition satisfied, proceed to next section.
    // NOTE: you can call 'NextSection' anywhere within 'Operation'
    if (counter == 10)
    {
        DEBUG_PRINT ("proceed to next section");
        GetScript()->NextSection ();
    }
}
