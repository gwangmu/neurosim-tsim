#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

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
    PORT_SELECT = CreatePort ("select_mux", Module::PORT_OUTPUT, Prototype<IntegerMessage>::Get());

    recvdata = 0;
    cursrc = 0;
}

// NOTE: called only if not stalled
void DataSinkModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    ExampleMessage *inmsg = static_cast<ExampleMessage *>(inmsgs[PORT_DATAIN]);
        
    if (recvdata % 3 == 0)
    {
        DEBUG_PRINT ("switching select to %d", !cursrc);
        cursrc = !cursrc;
        outmsgs[PORT_SELECT] = new IntegerMessage (cursrc);
    }

    if (inmsg) 
    {
        recvdata = inmsg->value;
        DEBUG_PRINT ("val = %u,", recvdata);
    }
}
