#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/NeuroChip.h>

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

NeuroSim::NeuroSim (string iname, Component *parent)
    : Component ("NeuroSim", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    /** Components **/
    Component *neurochip = new NeuroChip ("chip0", this);
   
    /** Modules **/

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *datasink = new DataSinkModule <AxonMessage, uint32_t> ("datasink", this);

    // Wires
    Wire *core2sink = new Wire (this, conattr, Prototype<AxonMessage>::Get()); // DUMMY

    /** Connect **/
    neurochip->Connect ("Axon", core2sink->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", core2sink->GetEndpoint (Endpoint::RHS));

}

