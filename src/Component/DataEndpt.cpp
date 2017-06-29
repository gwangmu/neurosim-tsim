#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataEndpt.h>
#include <Script/ExampleFileScript.h>
#include <Script/ExampleInstruction.h>
#include <Message/ExampleMessage.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

template <class M>
DataEndptModule<M>::DataEndptModule (string iname, Component *parent)
    : Module ("DataEndptModule", iname, parent, 1)
{
    // create ports
    PORT_DATAOUT = CreatePort ("dataend",
            Module::PORT_OUTPUT, Prototype<M>::Get());

}

template class DataEndptModule <IndexMessage>;
template class DataEndptModule <StateMessage>;
template class DataEndptModule <DeltaGMessage>;
template class DataEndptModule <SignalMessage>;
