#include <Component/NeuroCore.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>

#include <Component/Accumulator.h>
#include <Component/AxonMetaQueue.h>
#include <Component/AxonMetaTable.h>
#include <Component/NeuronBlock.h>
#include <Component/NBController.h>
#include <Component/CoreTSMgr.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>
#include <Component/SynDataQueue.h>

#include <Component/SRAMModule.h>
#include <Component/StateSRAM.h>
#include <Component/DeltaSRAM.h>

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

NeuroCore::NeuroCore (string iname, Component *parent, int num_propagators)
    : Component ("NeuroCore", iname, parent)
{
    // Parameters
    int syn_queue_size = 64;
    int pipeline_depth = 2;
    int num_neurons = GET_PARAMETER (num_neurons);

    Module *neuron_block = new NeuronBlock ("neuron_block", this, pipeline_depth);
    Module *nb_controller = new NBController ("nb_controller", this, num_neurons);
   
    Module *state_sram = new StateSRAM ("state_sram", this, 64, 16);
    Module *deltaG_storage = new DeltaStorage ("delta_storage", this, 64, 16); 

    Module *core_tsmgr = new CoreTSMgr ("core_tsmgr", this);

    Module *axon_queue = new AxonMetaQueue ("axon_meta_queue", this);
    Module *axon_table = new AxonMetaTable ("axon_meta_table", this, num_neurons);

    Module *accumulator = new Accumulator ("accumulator", this);

    std::vector<Module*> syn_queue;
    for(int i=0; i<num_propagators; i++)
        syn_queue.push_back (new SynDataQueue ("syn_data_queue" + to_string(i), this, syn_queue_size));

    AndGate *empty_and = new AndGate ("empty_and", this, num_propagators);

    /*******************************************************************************/
    /** Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Neuron Block Controller 
    Wire *ctrl2nb = new Wire (this, conattr, Prototype<NeuronBlockInMessage>::Get());
    Wire *core_ssram_raddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *nbc_dsram_raddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *nbc_end = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    
    // Neuron Block
    Wire *state_waddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *state_wdata = new Wire (this, conattr, Prototype<StateMessage>::Get());
    Wire *nb_idle = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *dyn_result = 
        new Wire (this, conattr, Prototype<NeuronBlockOutMessage>::Get());

    // Axon Metadata Queue
    Wire *am_table_addr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *amq_empty = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    // Axon Metadata Table
    Wire *am_table_data = new Wire (this, conattr, Prototype<AxonMessage>::Get());

    // State SRAM
    Wire *state_sram_rdata = new Wire (this, conattr, Prototype<StateMessage>::Get());

    // Delta G SRAM
    Wire *deltaG_nbc_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
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
    RRFaninWire *synapse_info = new RRFaninWire (this, conattr, Prototype<SynapseMessage>::Get(), num_propagators);

    Wire *sdq_empty_and = new Wire (this, conattr, Prototype<IntegerMessage>::Get());

    // Core Timestep Manager
    FanoutWire *core_TSParity = 
        new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), 3 + num_propagators);

    /*******************************************************************************/
    /*** Connect modules ***/
    // Neuron block controller
    nb_controller->Connect ("deltaG", deltaG_nbc_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("state", state_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("tsparity", core_TSParity->GetEndpoint (Endpoint::RHS, 0)); 
    nb_controller->Connect ("neuron_block", ctrl2nb->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("ssram", core_ssram_raddr->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("dsram", nbc_dsram_raddr->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("end", nbc_end->GetEndpoint (Endpoint::LHS)); 
    
    // Neuron block
    neuron_block->Connect ("NeuronBlock_in", ctrl2nb->GetEndpoint (Endpoint::RHS));
    neuron_block->Connect ("NeuronBlock_out", dyn_result->GetEndpoint (Endpoint::LHS)); 
    neuron_block->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("idle", nb_idle->GetEndpoint (Endpoint::LHS)); 
       
    // Axon Metadata Queue
    axon_queue->Connect ("nb", dyn_result->GetEndpoint (Endpoint::RHS));
    axon_queue->Connect ("meta", am_table_data->GetEndpoint (Endpoint::RHS));
    axon_queue->Connect ("sram", am_table_addr->GetEndpoint (Endpoint::LHS));
    axon_queue->Connect ("empty", amq_empty->GetEndpoint (Endpoint::LHS)); 

    // Axon Metadata Table
    axon_table->Connect ("r_addr", am_table_addr->GetEndpoint (Endpoint::RHS));
    axon_table->Connect ("r_data", am_table_data->GetEndpoint (Endpoint::LHS));

    // State SRAM
    state_sram->Connect ("r_addr", core_ssram_raddr->GetEndpoint (Endpoint::RHS, 0)); 
    state_sram->Connect ("r_data", state_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    state_sram->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::RHS));
    state_sram->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::RHS));
    state_wdata->GetEndpoint (Endpoint::RHS) ->SetCapacity (10);
    state_waddr->GetEndpoint (Endpoint::RHS) ->SetCapacity (10);

    // Delta G SRAM
    deltaG_storage->Connect ("nr_addr", nbc_dsram_raddr->GetEndpoint (Endpoint::RHS)); 
    deltaG_storage->Connect ("nr_data", deltaG_nbc_rdata->GetEndpoint (Endpoint::LHS)); 
    deltaG_storage->Connect ("ar_addr", acc_dsram_raddr->GetEndpoint (Endpoint::RHS)); 
    deltaG_storage->Connect ("ar_data", deltaG_acc_rdata->GetEndpoint (Endpoint::LHS)); 
    deltaG_storage->Connect ("w_addr", deltaG_waddr->GetEndpoint (Endpoint::RHS)); // portcap
    deltaG_storage->Connect ("w_data", deltaG_wdata->GetEndpoint (Endpoint::RHS)); // portcap
    deltaG_storage->Connect ("TSparity", core_TSParity->GetEndpoint (Endpoint::RHS, 1)); // portcap
    deltaG_nbc_rdata->GetEndpoint (Endpoint::RHS)->SetCapacity(3);
    deltaG_acc_rdata->GetEndpoint (Endpoint::RHS)->SetCapacity(3);

    // Core Timestep Manager
    core_tsmgr->Connect ("NBC_end", nbc_end->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("NB_idle", nb_idle->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("AMQ_empty", amq_empty->GetEndpoint (Endpoint::RHS));
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
        //synapse_info->GetEndpoint (Endpoint::LHS, i)->SetCapacity (syn_queue_size);
        synapse_info->GetEndpoint (Endpoint::LHS, i)->SetCapacity (4);

        empty_and->Connect("input" + to_string(i), sdq_empty[i]->GetEndpoint (Endpoint::RHS));
    }
    
    empty_and->Connect("output", sdq_empty_and->GetEndpoint (Endpoint::LHS));

    /*** Export port ***/    
    //ExportPort ("Core_out", neuron_block, "NeuronBlock_out");
    ExportPort ("AxonData", axon_queue, "axon");
    ExportPort ("CurTSParity", core_tsmgr, "curTS");
    ExportPort ("DynFin", core_tsmgr, "DynFin");
    
    for(int i=0; i<num_propagators; i++)
    {
        ExportPort ("SynData" + to_string(i), syn_queue[i], "syn");
        ExportPort ("SynTS" + to_string(i), syn_queue[i], "syn_ts");
    }
}










