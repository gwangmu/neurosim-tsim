#include <Component/NeuroChip.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/NeuroCore.h>
#include <Component/SynDataDistrib.h>
#include <Component/AxonTransmitter.h>

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

NeuroChip::NeuroChip (string iname, Component *parent, int num_propagators)
    : Component ("NeuroChip", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    /** Parameters **/
    int num_cores = 1;

    /** Components **/
    std::vector<Component*> cores;
    for (int i=0; i<num_cores; i++)
        cores.push_back (new NeuroCore ("core" + to_string(i) , this));

    //Component *neurocore = new NeuroCore ("core0", this);
   
    /** Modules **/
    Module *axon_transmitter = new AxonTransmitter ("axon_transmitter", this);
    Module *syn_distributor = new SynDataDistrib ("syn_distributor", this, num_propagators);

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Modules
    Module *ds_dynfin = new DataSinkModule <SignalMessage, bool> ("ds_dynfin", this);

    // Wires
    std::vector<Wire*> core_DynFin;
    for (int i =0; i<num_cores; i++)
        core_DynFin.push_back (new Wire (this, conattr, Prototype<SignalMessage>::Get()));
    
    std::vector<Wire*> syn_data;
    std::vector<Wire*> syn_parity;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (new Wire (this, conattr, Prototype<SynapseMessage>::Get()));
        syn_parity.push_back (new Wire (this, conattr, Prototype<SignalMessage>::Get()));
    }
  
    Wire *axon_data = new Wire (this, conattr, Prototype<AxonMessage>::Get());

    /** Connect **/
    for (int i=0; i<num_cores; i++)
    {
        cores[i]->Connect ("DynFin", core_DynFin[i]->GetEndpoint (Endpoint::LHS));
        ds_dynfin->Connect ("datain", core_DynFin[i]->GetEndpoint (Endpoint::RHS));

        cores[i]->Connect ("AxonData", axon_data->GetEndpoint (Endpoint::LHS));
        axon_transmitter->Connect ("axon_in", axon_data->GetEndpoint (Endpoint::RHS));
        
        for (int j=0; i<num_propagators; i++)
        {
            syn_distributor->Connect ("syn_out" + to_string(j), syn_data[j]->GetEndpoint (Endpoint::LHS));
            cores[i]->Connect ("SynData", syn_data[j]->GetEndpoint (Endpoint::RHS));

            syn_distributor->Connect ("syn_ts_out" + to_string(j), syn_parity[j]->GetEndpoint (Endpoint::LHS));
            cores[i]->Connect ("SynTS", syn_parity[j]->GetEndpoint (Endpoint::RHS));
        }
    }
    
    ExportPort ("Axon", axon_transmitter, "axon_out");
    ExportPort ("CurTSParity", cores[0], "CurTSParity");

    for (int i=0; i<num_propagators; i++)
    {
        ExportPort ("SynapseData" + to_string(i), syn_distributor, "syn_in" + to_string(i));
        ExportPort ("SynCidx" + to_string(i), syn_distributor, "syn_cidx" + to_string(i));
        ExportPort ("SynTS" + to_string(i), syn_distributor, "syn_ts_in" + to_string(i));
    }
}

