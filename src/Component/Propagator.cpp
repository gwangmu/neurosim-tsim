#include <Component/Propagator.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Component/AxonMetaRecv.h>
#include <Component/AxonStreamer.h>
#include <Component/AxonClassifier.h>
#include <Component/AxonStorage.h>
#include <Component/PseudoStorage.h>
#include <Component/DelayModule.h>
#include <Component/FastDelayMgr.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/DramReqMessage.h>
#include <Message/DramMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

Propagator::Propagator (string iname, Component *parent, 
        int board_idx, int prop_idx)
    : Component ("Propagator", iname, parent)
{
    /** Parameters **/
    int dram_size = 4096;
    int dram_io_buf_size = 8;
    int dram_outque_size = 64 * dram_io_buf_size;

    int delay_storage_size = 2048;
    int delay_input_queue_sz = 256;

    int fast = GET_PARAMETER (fast);

    /** Components **/
    // Component *delay_module =
    //     new DelayModule ("delay_module", this);
    
    Module *delay_module = new FastDelayMgr ("delay_module", this, board_idx); 
   
    /** Modules **/
    Module *axon_receiver = new AxonMetaRecv ("axon_meta_receiver", this);
    Module *axon_streamer = 
        new AxonStreamer ("axon_streamer", this, dram_io_buf_size);

    Module *axon_classifier = new AxonClassifier ("axon_classifier", this);

    Module *axon_storage;
    bool pseudo = GET_PARAMETER(pseudo);
    if(pseudo)
    {
        axon_storage = 
            new PseudoStorage ("axon_storage", this, 
                               dram_io_buf_size, dram_outque_size, 
                               board_idx, prop_idx); 
    }
    else
    {
        axon_storage = 
            new AxonStorage ("axon_storage", this, 
                             dram_io_buf_size, dram_outque_size);
    }

    
    AndGate *prop_idle = new AndGate ("prop_idle", this, 5); 

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    Wire *axon_data = new Wire (this, conattr, Prototype<AxonMessage>::Get());
    Wire *dram_addr = 
        new Wire (this, conattr, Prototype<DramReqMessage>::Get());
    Wire *dram_data = new Wire (this, conattr, Prototype<DramMessage>::Get());

    RRFaninWire *delay_out = 
        new RRFaninWire (this, conattr, Prototype<AxonMessage>::Get(), 2);
        
    Wire *recv_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *class_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *stream_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *dram_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());
    Wire *delay_idle = 
        new Wire (this, conattr, Prototype<IntegerMessage>::Get());
        
    /** Connect **/
    axon_receiver->Connect ("axon_out", axon_data->GetEndpoint (Endpoint::LHS));
    axon_streamer->Connect ("axon_in", axon_data->GetEndpoint (Endpoint::RHS)); 

    axon_streamer->Connect ("addr_out", dram_addr->GetEndpoint (Endpoint::LHS));
    axon_storage->Connect ("r_addr", dram_addr->GetEndpoint (Endpoint::RHS));
   
    axon_storage->Connect ("r_data", dram_data->GetEndpoint (Endpoint::LHS));
    axon_classifier->Connect ("dram", dram_data->GetEndpoint (Endpoint::RHS));
    dram_data->GetEndpoint (Endpoint::LHS)->SetCapacity (dram_outque_size);

    axon_classifier->Connect ("delay_out", 
                              delay_out->GetEndpoint (Endpoint::LHS, 0));
    axon_receiver->Connect ("delay_out", 
                              delay_out->GetEndpoint (Endpoint::LHS, 1));
    delay_module->Connect ("Input", 
                           delay_out->GetEndpoint (Endpoint::RHS));
    delay_out->GetEndpoint (Endpoint::RHS)->SetCapacity (delay_input_queue_sz);
    delay_out->GetEndpoint (Endpoint::LHS, 0)
             ->SetCapacity (2);

    axon_receiver->Connect ("idle", recv_idle->GetEndpoint (Endpoint::LHS)); 
    axon_classifier->Connect ("idle", class_idle->GetEndpoint (Endpoint::LHS)); 
    axon_streamer->Connect ("idle", stream_idle->GetEndpoint (Endpoint::LHS));
    axon_storage->Connect ("idle", dram_idle->GetEndpoint (Endpoint::LHS));
    delay_module->Connect ("Idle", 
                           delay_idle->GetEndpoint (Endpoint::LHS));

    prop_idle->Connect ("input0", recv_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input1", class_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input2", stream_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input3", dram_idle->GetEndpoint (Endpoint::RHS));
    prop_idle->Connect ("input4", delay_idle->GetEndpoint (Endpoint::RHS));

    ExportPort ("Axon", axon_receiver, "axon_in"); 
    ExportPort ("PropTS", axon_classifier, "ts_parity");
    
    ExportPort ("DelayTS", delay_module, "TSParity"); 
    ExportPort ("DelayAxon", delay_module, "Output");

    ExportPort ("Synapse", axon_classifier, "syn_out");
    ExportPort ("SynTS", axon_classifier, "ts_out");
    ExportPort ("BoardAxon", axon_classifier, "axon_out");
    ExportPort ("Index", axon_classifier, "tar_idx");
    ExportPort ("BoardID", axon_classifier, "board_id");
    //ExportPort ("Idle", axon_classifier, "idle");
    ExportPort ("Idle", prop_idle, "output");
}



