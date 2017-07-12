#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataSourceModule.h>
#include <Message/ExamplePCIeMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSourceModule::DataSourceModule (string iname, Component *parent, uint32_t testid)
    : Module ("DataSourceModule", iname, parent, 1)
{
    // create ports
    PORT_DATAOUT = CreatePort ("out", Module::PORT_OUTPUT, 
            Prototype<ExamplePCIeMessage>::Get());
    PORT_DATAIN = CreatePort ("in", Module::PORT_INPUT,
            Prototype<ExamplePCIeMessage>::Get());

    this->testid = testid;
    counter = 0;
}

void DataSourceModule::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    if (inmsgs[PORT_DATAIN])
    {
        ExamplePCIeMessage *pciemsg = 
            static_cast<ExamplePCIeMessage *>(inmsgs[PORT_DATAIN]);

        PRINT ("%s received PCIe message %p (from: %u, value: %u)",
                GetName().c_str(), pciemsg, pciemsg->from, pciemsg->value);
    }

    if (counter % MAX_TEST_SOURCES == testid)
    {
        outmsgs[PORT_DATAOUT] = new ExamplePCIeMessage (
                testid, counter, (testid + 1) % MAX_TEST_SOURCES);

        PRINT ("%s sended PCIe message %p (to: %u, value: %u)",
                GetName().c_str(), outmsgs[PORT_DATAOUT], (testid + 1) % MAX_TEST_SOURCES, counter);
    }
    
    counter++;
}
