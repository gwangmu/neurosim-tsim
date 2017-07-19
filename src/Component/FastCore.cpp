#include <Component/FastCore.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>

#include <Component/Accumulator.h>
#include <Component/FastCoreTSMgr.h>
#include <Component/SynDataQueue.h>
#include <Component/CoreDynUnit.h>

#include <Component/SimpleDelta.h>

#include <Message/AxonMessage.h>
#include <Message/ExampleMessage.h>
#include <Message/NeuronBlockMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SynapseMessage.h>
#include <Message/IndexMessage.h>
#include <Message/NeuronBlockMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

FastCore::FastCore (string iname, Component *parent, int num_propagators)
    : Component ("FastCore", iname, parent)
{
    // Parameters
    int syn_queue_size = 64;
    int pipeline_depth = 2;
    int num_neurons = GET_PARAMETER (num_neurons);

    // NBC-SRAM-NBC-(NB(depth))-AMQ-AT-AMQ
    pipeline_depth += 6;
    Module *dyn_unit =
        new CoreDynUnit ("core_dyn_unit", this, num_neurons, pipeline_depth);

    Module *core_tsmgr = new FastCoreTSMgr ("core_tsmgr", this);
    Module *accumulator = new Accumulator ("accumulator", this);
    Module *deltaG_storage = new SimpleDelta ("delta_storage", this, 64, 16); 

    std::vector<Module*> syn_queue;
    for(int i=0; i<num_propagators; i++)
        syn_queue.push_back (new SynDataQueue 
                ("syn_data_queue" + to_string(i), this, syn_queue_size));

    AndGate *empty_and = new AndGate ("empty_and", this, num_propagators);

    /*******************************************************************************/
    /** Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Dynamics unit 
    Wire *dyn_end = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    // Delta G SRAM
    //Wire *deltaG_nbc_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
    Wire *deltaG_acc_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
    
    // Accumulator
    Wire *acc_idle = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *acc_dsram_raddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *deltaG_waddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *deltaG_wdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());

    // Synapse Data Queue
    std::vector<Wire*> sdq_empty;
    for (int i=0; i<num_propagators; i++)
        sdq_empty.push_back (new Wire (this, conattr, Prototype<IntegerMessage>::Get()));
    RRFaninWire *synapse_info = 
        new RRFaninWire 
            (this, conattr, Prototype<SynapseMessage>::Get(), num_propagators);

    Wire *sdq_empty_and = new Wire (this, conattr, Prototype<IntegerMessage>::Get());

    // Core Timestep Manager
    FanoutWire *core_TSParity = 
        new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), 3 + num_propagators);

    /*******************************************************************************/
    /*** Connect modules ***/
    // Neuron block controller
    dyn_unit->Connect ("coreTS", core_TSParity->GetEndpoint (Endpoint::RHS, 0)); 
    dyn_unit->Connect ("dynfin", dyn_end->GetEndpoint (Endpoint::LHS)); 
    

    // Delta G SRAM
    deltaG_storage->Connect ("ar_addr", acc_dsram_raddr->GetEndpoint (Endpoint::RHS)); 
    deltaG_storage->Connect ("ar_data", deltaG_acc_rdata->GetEndpoint (Endpoint::LHS)); 
    deltaG_storage->Connect ("w_addr", deltaG_waddr->GetEndpoint (Endpoint::RHS)); // portcap
    deltaG_storage->Connect ("w_data", deltaG_wdata->GetEndpoint (Endpoint::RHS)); // portcap
    deltaG_storage->Connect ("TSparity", core_TSParity->GetEndpoint (Endpoint::RHS, 1)); // portcap
    deltaG_acc_rdata->GetEndpoint (Endpoint::RHS)->SetCapacity(3);

    // Core Timestep Manager
    core_tsmgr->Connect ("dyn_end", dyn_end->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("Acc_idle", acc_idle->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("SDQ_empty", sdq_empty_and->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("Tsparity", core_TSParity->GetEndpoint (Endpoint::LHS));
   
    // Accumulator
    accumulator->Connect("synapse", synapse_info->GetEndpoint (Endpoint::RHS));
    accumulator->Connect("tsparity", core_TSParity->GetEndpoint (Endpoint::RHS, 2));
    accumulator->Connect("r_addr", acc_dsram_raddr->GetEndpoint (Endpoint::LHS)); 
    accumulator->Connect("r_data", deltaG_acc_rdata->GetEndpoint (Endpoint::RHS)); 
    accumulator->Connect("w_addr", deltaG_waddr->GetEndpoint (Endpoint::LHS));
    accumulator->Connect("w_data", deltaG_wdata->GetEndpoint (Endpoint::LHS));
    accumulator->Connect("idle", acc_idle->GetEndpoint (Endpoint::LHS)); 

    // Synapse data queue
    for (int i=0; i<num_propagators; i++)
    {
        syn_queue[i]->Connect("core_ts", core_TSParity->GetEndpoint (Endpoint::RHS, 3 + i)); 
        syn_queue[i]->Connect("empty", sdq_empty[i]->GetEndpoint (Endpoint::LHS));
        syn_queue[i]->Connect("acc", synapse_info->GetEndpoint (Endpoint::LHS, i));
        synapse_info->GetEndpoint (Endpoint::LHS, i)->SetCapacity (syn_queue_size);

        empty_and->Connect("input" + to_string(i), sdq_empty[i]->GetEndpoint (Endpoint::RHS));
    }
    
    empty_and->Connect("output", sdq_empty_and->GetEndpoint (Endpoint::LHS));

    /*** Export port ***/    
    //ExportPort ("Core_out", neuron_block, "NeuronBlock_out");
    ExportPort ("AxonData", dyn_unit, "axon");
    ExportPort ("CurTSParity", core_tsmgr, "curTS");
    ExportPort ("DynFin", core_tsmgr, "DynFin");
    
    for(int i=0; i<num_propagators; i++)
    {
        ExportPort ("SynData" + to_string(i), syn_queue[i], "syn");
        ExportPort ("SynTS" + to_string(i), syn_queue[i], "syn_ts");
    }
}










