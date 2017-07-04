#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Script/ExampleFileScript.h>
#include <Component/DataSinkModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSinkModule::DataSinkModule (string iname, Component *parent)
    : Module ("DataSinkModule", iname, parent, 1)
{
    // create ports
    PORT_DATAIN = CreatePort ("datain", Module::PORT_INPUT, Prototype<IntegerMessage>::Get());

    recvdata = 0;
}

// NOTE: called only if not stalled
void DataSinkModule::Operation (Message **inmsgs, Message **outmsgs, const uint32_t *outque_size, Instruction *instr)
{
    IntegerMessage *inmsg = static_cast<IntegerMessage *>(inmsgs[PORT_DATAIN]);
        
    if (inmsg) 
    {
        recvdata = inmsg->value;
        DEBUG_PRINT ("val = %x,", recvdata);
    }
}
