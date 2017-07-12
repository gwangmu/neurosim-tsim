#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Component/DataSinkModule.h>

#include <Message/AxonMessage.h>
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


template <class M, class T>
DataSinkModule<M, T>::DataSinkModule (string iname, Component *parent)
    : Module ("DataSinkModule", iname, parent, 1)
{
    // create ports
    PORT_DATAIN = CreatePort ("datain", Module::PORT_INPUT, Prototype<M>::Get());

    recvdata = 0;
}

// NOTE: called only if not stalled
template <class M, class T>
void DataSinkModule<M, T>::Operation (Message **inmsgs, Message **outmsgs,
        const uint32_t *outque_size, Instruction *instr)
{
    M *inmsg = static_cast<M*>(inmsgs[PORT_DATAIN]);

    if (inmsg) 
    {
        recvdata = inmsg->value;
        INFO_PRINT ("[Sink] val = %u", recvdata);
    }
}

template class DataSinkModule <NeuronBlockOutMessage, uint32_t>;
template class DataSinkModule <SignalMessage, bool>;
template class DataSinkModule <AxonMessage, uint32_t>;
