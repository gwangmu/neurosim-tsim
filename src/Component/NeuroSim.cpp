#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/NeuronBlock.h>
#include <Component/NBController.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/SRAMModule.h>
#include <Component/StateSRAM.h>
#include <Component/DeltaSRAM.h>

#include <Component/NeuroCore.h>

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
    Component *neurocore = new NeuroCore ("core0", this);
    
    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *datasink = new DataSinkModule <NeuronBlockOutMessage, uint32_t> ("datasink", this);
    Module *ds_dynfin = new DataSinkModule <SignalMessage, bool> ("ds_dynfin", this);
    //Module *ds_parity = new DataSinkModule <SignalMessage, bool> ("ds_parity", this);
    
    // Wires
    Wire *nb2sink = new Wire (this, conattr, Prototype<NeuronBlockOutMessage>::Get());
    Wire *core_DynFin = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    //Wire *core_TSParity = new Wire (this, conattr, Prototype<SignalMessage>::Get());
   
    /** Connect **/
    neurocore->Connect ("Core_out", nb2sink->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", nb2sink->GetEndpoint (Endpoint::RHS));

    neurocore->Connect ("DynFin", core_DynFin->GetEndpoint (Endpoint::LHS));
    ds_dynfin->Connect ("datain", core_DynFin->GetEndpoint (Endpoint::RHS));
    
    //de_parity->Connect ("dataend", core_TSParity->GetEndpoint (Endpoint::LHS));
    //neurocore->Connect ("TSParity", core_TSParity->GetEndpoint (Endpoint::RHS));
}

