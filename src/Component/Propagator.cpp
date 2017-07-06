#include <Component/Propagator.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/AxonMetaRecv.h>
#include <Component/AxonStreamer.h>
#include <Component/AxonClassifier.h>
#include <Component/AxonStorage.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/DramMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

Propagator::Propagator (string iname, Component *parent)
    : Component ("Propagator", iname, parent)
{
    /** Parameters **/
        
    /** Components **/
   
    /** Modules **/
    Module *axon_receiver = new AxonMetaRecv ("axon_meta_receiver", this);
    Module *axon_streamer = new AxonStreamer ("axon_streamer", this);

    Module *axon_classifier = new AxonClassifier ("axon_classifier", this);
    Module *axon_storage = new AxonStorage ("axon_storage", this); 

    AndGate *prop_idle = new AndGate ("prop_idle", this, 4); 

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    

    // Wires
    Wire *axon_data = new Wire (this, conattr, Prototype<AxonMessage>::Get());
    Wire *dram_addr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *dram_data = new Wire (this, conattr, Prototype<DramMessage>::Get());

    Wire *recv_idle = new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *class_idle = new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *stream_idle = new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *dram_idle = new Wire (this, conattr, Prototype<IntegerMessage>::Get());
        
    /** Connect **/
    axon_receiver->Connect ("axon_out", axon_data->GetEndpoint (Endpoint::LHS));
    axon_streamer->Connect ("axon_in", axon_data->GetEndpoint (Endpoint::RHS)); 

    axon_streamer->Connect ("addr_out", dram_addr->GetEndpoint (Endpoint::LHS));
    axon_storage->Connect ("r_addr", dram_addr->GetEndpoint (Endpoint::RHS));
   
    axon_storage->Connect ("r_data", dram_data->GetEndpoint (Endpoint::LHS));
    axon_classifier->Connect ("dram", dram_data->GetEndpoint (Endpoint::RHS));

    axon_receiver->Connect ("idle", recv_idle->GetEndpoint (Endpoint::LHS)); 
    axon_classifier->Connect ("idle", class_idle->GetEndpoint (Endpoint::LHS)); 
    axon_streamer->Connect ("idle", stream_idle->GetEndpoint (Endpoint::LHS));
    axon_storage->Connect ("idle", dram_idle->GetEndpoint (Endpoint::LHS));

    prop_idle->Connect ("input0", recv_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input1", class_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input2", stream_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input3", dram_idle->GetEndpoint (Endpoint::RHS));

    ExportPort ("Axon", axon_receiver, "axon_in"); 
    ExportPort ("PropTS", axon_classifier, "ts_parity");

    ExportPort ("Synapse", axon_classifier, "syn_out");
    ExportPort ("SynTS", axon_classifier, "ts_out");
    ExportPort ("BoardAxon", axon_classifier, "axon_out");
    ExportPort ("Index", axon_classifier, "tar_idx");
    ExportPort ("BoardID", axon_classifier, "board_id");
    //ExportPort ("Idle", axon_classifier, "idle");
    ExportPort ("Idle", prop_idle, "output");
}



