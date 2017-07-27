#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataEndpt.h>
#include <Script/ExampleFileScript.h>
#include <Script/ExampleInstruction.h>
#include <Message/ExampleMessage.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SynapseMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

template <class M>
class DataEndptModule: public Module
{
public:
    DataEndptModule (string iname, Component *parent);
    virtual void Operation (Message **inmsgs, Message **outmsgs, Instruction *instr) {}

private:
    // Port IDs
    uint32_t PORT_DATAOUT;
};

template <class M>
DataEndptModule<M>::DataEndptModule (string iname, Component *parent)
    : Module ("DataEndptModule", iname, parent, 1)
{
    // create ports
    PORT_DATAOUT = CreatePort ("dataend",
            Module::PORT_OUTPUT, Prototype<M>::Get());

}
