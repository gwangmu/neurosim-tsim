#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataSourceModule.h>
#include <Message/ExampleRemoteMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


DataSourceModule::DataSourceModule (string iname, Component *parent, string clock, uint32_t testid)
    : Module ("DataSourceModule", iname, parent, 1)
{
    SetClock (clock);

    // create ports
    PORT_DATAOUT = CreatePort ("out", Module::PORT_OUTPUT, 
            Prototype<ExampleRemoteMessage>::Get());
    PORT_DATAIN = CreatePort ("in", Module::PORT_INPUT,
            Prototype<ExampleRemoteMessage>::Get());

    this->testid = testid;
    counter = 0;
}

void DataSourceModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    if (inmsgs[PORT_DATAIN])
    {
        ExampleRemoteMessage *pciemsg = 
            static_cast<ExampleRemoteMessage *>(inmsgs[PORT_DATAIN]);

        PRINT ("%s received Remote message %p (from: %u, value: %u)",
                GetName().c_str(), pciemsg, pciemsg->from, pciemsg->value);
    }

    if (counter % (MAX_TEST_SOURCES/16) == testid/16)
    {
        outmsgs[PORT_DATAOUT] = new ExampleRemoteMessage (
                testid, counter, (testid + 1) % MAX_TEST_SOURCES);

        PRINT ("%s sended Remote message %p (to: %u, value: %u)",
                GetName().c_str(), outmsgs[PORT_DATAOUT], (testid + 1) % MAX_TEST_SOURCES, counter);
    }
    
    counter++;
}
