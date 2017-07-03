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
#include <Message/SelectMessage.h>
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

    /** Parameters **/
    int num_propagators = 1;

    /** Components **/
    Component *neurochip = new NeuroChip ("chip0", this, num_propagators);
   
    /** Modules **/

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *datasink = new DataSinkModule <AxonMessage, uint32_t> ("datasink", this);
    Module *ds_parity = new DataSourceModule ("ds_parity", this);
    
    Module *de_syn = new DataEndptModule <SynapseMessage> ("de_syn", this);
    Module *de_synTS = new DataEndptModule <SignalMessage> ("de_synTS", this);
    Module *de_synCidx = new DataEndptModule <SelectMessage> ("de_synCidx", this);

    // Wires
    Wire *core2sink = new Wire (this, conattr, Prototype<AxonMessage>::Get()); // DUMMY
    Wire *core_TSParity = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    
    std::vector<Wire*> syn_data;
    std::vector<Wire*> syn_parity;
    std::vector<Wire*> syn_cidx;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (new Wire (this, conattr, Prototype<SynapseMessage>::Get()));
        syn_parity.push_back (new Wire (this, conattr, Prototype<SignalMessage>::Get()));
        syn_cidx.push_back (new Wire (this, conattr, Prototype<SelectMessage>::Get()));
    }
    
    /** Connect **/
    neurochip->Connect ("Axon", core2sink->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", core2sink->GetEndpoint (Endpoint::RHS));

    ds_parity->Connect ("dataout", core_TSParity->GetEndpoint (Endpoint::LHS));
    neurochip->Connect ("CurTSParity", core_TSParity->GetEndpoint (Endpoint::RHS));

    for (int i=0; i<num_propagators; i++)
    {
        de_syn->Connect ("dataend", syn_data[i]->GetEndpoint (Endpoint::LHS));
        neurochip->Connect ("SynapseData" + to_string(i), syn_data[i]->GetEndpoint (Endpoint::RHS));

        de_synTS->Connect ("dataend", syn_parity[i]->GetEndpoint (Endpoint::LHS));
        neurochip->Connect ("SynTS" + to_string(i), syn_parity[i]->GetEndpoint (Endpoint::RHS));

        de_synCidx->Connect ("dataend", syn_cidx[i]->GetEndpoint (Endpoint::LHS));
        neurochip->Connect ("SynCidx" + to_string(i), syn_cidx[i]->GetEndpoint (Endpoint::RHS));
    }
}



