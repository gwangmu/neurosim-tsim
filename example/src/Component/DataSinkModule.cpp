#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Script/ExampleFileScript.h>
#include <Component/DataSinkModule.h>
#include <Message/ExampleMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSinkModule::DataSinkModule (string iname, Component *parent)
    : Module ("DataSinkModule", iname, parent, 1)
{
    // create ports
    PORT_DATAIN = CreatePort ("datain", Module::PORT_INPUT, Prototype<ExampleMessage>::Get());

    recvdata = 0;
}

// NOTE: called only if not stalled
void DataSinkModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    ExampleMessage *inmsg = static_cast<ExampleMessage *>(inmsgs[PORT_DATAIN]);

    if (inmsg) 
    {
        recvdata = inmsg->value;
        DEBUG_PRINT ("value = %u", recvdata);
    }
}
