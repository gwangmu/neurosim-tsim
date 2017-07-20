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
#include <Component/CoreAccUnit.h>
#include <Component/FastSynQueue.h>

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
    Module *acc_unit =
        new CoreAccUnit ("core_acc_unit", this, num_propagators);
    Module *core_tsmgr = new FastCoreTSMgr ("core_tsmgr", this);

    /**************************************************************************/
    /** Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Dynamics unit 
    Wire *dyn_end = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    // Accumulator
    Wire *acc_idle = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    // Core Timestep Manager
    FanoutWire *core_TSParity = 
        new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), 2);

    /**************************************************************************/
    /*** Connect modules ***/
    // Core dynamics unit
    dyn_unit->Connect ("coreTS", 
                       core_TSParity->GetEndpoint (Endpoint::RHS, 0)); 
    dyn_unit->Connect ("dynfin", dyn_end->GetEndpoint (Endpoint::LHS)); 
    
    // Core accumulation unit
    acc_unit->Connect ("coreTS", 
                       core_TSParity->GetEndpoint (Endpoint::RHS, 1)); 
    acc_unit->Connect ("accfin", acc_idle->GetEndpoint (Endpoint::LHS)); 

    // Core Timestep Manager
    core_tsmgr->Connect ("dyn_end", dyn_end->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("acc_idle", acc_idle->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("Tsparity", 
                         core_TSParity->GetEndpoint (Endpoint::LHS));
   

    /*** Export port ***/    
    //ExportPort ("Core_out", neuron_block, "NeuronBlock_out");
    ExportPort ("AxonData", dyn_unit, "axon");
    ExportPort ("CurTSParity", core_tsmgr, "curTS");
    ExportPort ("DynFin", core_tsmgr, "DynFin");
    
    for(int i=0; i<num_propagators; i++)
    {
        ExportPort ("SynData" + to_string(i), acc_unit, "syn" + to_string(i));
        ExportPort ("SynTS" + to_string(i), acc_unit, "synTS" + to_string(i));
    }
}





