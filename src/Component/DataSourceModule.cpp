#include <Component/DataSourceModule.h>
#include <Message/ExampleMessage.h>
#include <TSim/Utility/Prototype.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


DataSourceModule::DataSourceModule (string iname, Component *parent)
    : Module ("DataSourceModule", iname, parent)
{
    PORT_DATAOUT = CreatePort ("dataout", Module::PORT_OUTPUT, Prototype<ExampleMessage>::Get());

    counter = 0;
}

// NOTE: called only if not stalled
void DataSourceModule::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    outmsgs[PORT_DATAOUT] = new ExampleMessage (0, counter);
    counter++;
}
