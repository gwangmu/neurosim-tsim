#include <TSim/Simulation/Testbench.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Register/ExampleFileRegister.h>
#include <Register/ExampleRegisterWord.h>
#include <Script/ExampleFileScript.h>
#include <Component/DataSinkModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


USING_TESTBENCH;


DataSinkModule::DataSinkModule (string iname, Component *parent)
    : Module ("DataSinkModule", iname, parent, 1)
{
    // create ports
    PORT_DATAIN = CreatePort ("datain", Module::PORT_INPUT, Prototype<IntegerMessage>::Get());

    recvdata = GET_PARAMETER (param1);

    Register::Attr regattr (64, 1024);  // word=64 bits, addr=1024 words
    SetRegister (new ExampleFileRegister (Register::SRAM, regattr));
}

// NOTE: called only if not stalled
void DataSinkModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    IntegerMessage *inmsg = static_cast<IntegerMessage *>(inmsgs[PORT_DATAIN]);
        
    if (inmsg) 
    {
        recvdata = inmsg->value;
        DEBUG_PRINT ("val = %x,", recvdata);
        
        const ExampleRegisterWord *word = static_cast<const ExampleRegisterWord *>(GetRegister()->GetWord (0x0000));
        DEBUG_PRINT ("regval = %x", word->value);
        GetRegister()->SetWord (0x0000, new ExampleRegisterWord (recvdata));
    }
}
