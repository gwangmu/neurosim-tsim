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
    Component *neurocore = new NeuroCore ("core0", this);
    
    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *datasink = new DataSinkModule <AxonMessage, uint32_t> ("datasink", this);
    Module *ds_dynfin = new DataSinkModule <SignalMessage, bool> ("ds_dynfin", this);
    Module *ds_parity = new DataSourceModule ("ds_parity", this);
    Module *de_syn = new DataEndptModule <SynapseMessage> ("de_syn", this);
    Module *de_synTS = new DataEndptModule <SignalMessage> ("de_synTS", this);

    // Wires
    Wire *core2sink = new Wire (this, conattr, Prototype<AxonMessage>::Get());
    Wire *core_DynFin = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *core_TSParity = new Wire (this, conattr, Prototype<SignalMessage>::Get());
  
    Wire *syn_data = new Wire (this, conattr, Prototype<SynapseMessage>::Get());
    Wire *syn_parity = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    /** Connect **/
    neurocore->Connect ("AxonData", core2sink->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", core2sink->GetEndpoint (Endpoint::RHS));

    neurocore->Connect ("DynFin", core_DynFin->GetEndpoint (Endpoint::LHS));
    ds_dynfin->Connect ("datain", core_DynFin->GetEndpoint (Endpoint::RHS));
    
    ds_parity->Connect ("dataout", core_TSParity->GetEndpoint (Endpoint::LHS));
    neurocore->Connect ("CurTSParity", core_TSParity->GetEndpoint (Endpoint::RHS));

    de_syn->Connect ("dataend", syn_data->GetEndpoint (Endpoint::LHS));
    neurocore->Connect ("SynData", syn_data->GetEndpoint (Endpoint::RHS));

    de_synTS->Connect ("dataend", syn_parity->GetEndpoint (Endpoint::LHS));
    neurocore->Connect ("SynTS", syn_parity->GetEndpoint (Endpoint::RHS));
}

