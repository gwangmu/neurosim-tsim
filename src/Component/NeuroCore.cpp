#include <Component/NeuroCore.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/NeuronBlock.h>
#include <Component/NBController.h>
#include <Component/CoreTSMgr.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/SRAMModule.h>
#include <Component/StateSRAM.h>
#include <Component/DeltaSRAM.h>

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


NeuroCore::NeuroCore (string iname, Component *parent)
    : Component ("NeuroCore", iname, parent)
{
    // add child modules/components
    Module *neuron_block = new NeuronBlock ("neuron_block", this, 2);
    Module *nb_controller = new NBController ("nb_controller", this, 16);
   
    Module *state_sram = new StateSRAM ("state_sram", this, 64, 16);
    Module *deltaG_sram = new DeltaSRAM ("deltaG_sram", this, 64, 16);

    Module *core_tsmgr = new CoreTSMgr ("core_tsmgr", this);
    
    // Dummy modules
    Module *ds_reset = new DataSinkModule <SignalMessage, bool> ("ds_reset", this);

    Module *de_amq_empty = new DataEndptModule <SignalMessage> ("de_amq_empty", this);
    Module *de_acc_idle = new DataEndptModule <SignalMessage>  ("de_acc_idle", this);
    Module *de_sdq_empty = new DataEndptModule <SignalMessage> ("de_sdq_empty", this);
    Module *de_d_waddr = new DataEndptModule <IndexMessage> ("de_d_waddr", this);
    Module *de_d_wdata = new DataEndptModule <DeltaGMessage> ("de_d_wdata", this);
   

    /*******************************************************************************/
    /** Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Neuron Block
    Wire *ctrl2nb = new Wire (this, conattr, Prototype<NeuronBlockInMessage>::Get());

    Wire *state_waddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *state_wdata = new Wire (this, conattr, Prototype<StateMessage>::Get());

    // Neuron Block Controller
    FanoutWire *core_sram_raddr = new FanoutWire (this, conattr, Prototype<IndexMessage>::Get(), 2);

    Wire *state_sram_rdata = new Wire (this, conattr, Prototype<StateMessage>::Get());
    Wire *deltaG_sram_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());

    // Core Timestep Manager
    Wire *nbc_end = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *nb_idle = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *amq_empty = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *acc_idle = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *sdq_empty = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    
    Wire *deltaG_reset = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *core_TSParity = new Wire (this, conattr, Prototype<SignalMessage>::Get());// Fanout Wire

    // Dummy wire
    Wire *de_d_waddr_wire = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *de_d_wdata_wire = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
  

    /*******************************************************************************/
    /*** Connect modules ***/
    // Dummy Connection
    ds_reset->Connect ("datain", deltaG_reset->GetEndpoint (Endpoint::RHS));

    de_amq_empty->Connect ("dataend", amq_empty->GetEndpoint (Endpoint::LHS));
    de_acc_idle->Connect ("dataend", acc_idle->GetEndpoint (Endpoint::LHS));
    de_sdq_empty->Connect ("dataend", sdq_empty->GetEndpoint (Endpoint::LHS));
    de_d_waddr->Connect ("dataend", de_d_waddr_wire->GetEndpoint (Endpoint::LHS));
    de_d_wdata->Connect ("dataend", de_d_wdata_wire->GetEndpoint (Endpoint::LHS));

    // Neuron block controller
    nb_controller->Connect ("neuron_block", ctrl2nb->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("sram", core_sram_raddr->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("end", nbc_end->GetEndpoint (Endpoint::LHS)); 

    nb_controller->Connect ("deltaG", deltaG_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("state", state_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("tsparity", core_TSParity->GetEndpoint (Endpoint::RHS)); // conn w/ data src
        
    // State & Delta-G SRAM
    state_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS, 0)); 
    state_sram->Connect ("r_data", state_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    state_sram->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::RHS));
    state_sram->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::RHS));

    deltaG_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS, 1)); 
    deltaG_sram->Connect ("r_data", deltaG_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    deltaG_sram->Connect ("w_addr", de_d_waddr_wire->GetEndpoint (Endpoint::RHS)); // portcap
    deltaG_sram->Connect ("w_data", de_d_wdata_wire->GetEndpoint (Endpoint::RHS)); // portcap
    
    // Neuron block
    neuron_block->Connect ("NeuronBlock_in", ctrl2nb->GetEndpoint (Endpoint::RHS));
    //neuron_block->Connect ("NeuronBlock_out", nb2sink->GetEndpoint (Endpoint::LHS)); // outside
    neuron_block->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("idle", nb_idle->GetEndpoint (Endpoint::LHS)); 

    // Core Timestep Manager
    core_tsmgr->Connect ("NBC_end", nbc_end->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("NB_idle", nb_idle->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("AMQ_empty", amq_empty->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("Acc_idle", acc_idle->GetEndpoint (Endpoint::RHS));
    core_tsmgr->Connect ("SDQ_empty", sdq_empty->GetEndpoint (Endpoint::RHS));
    
    core_tsmgr->Connect ("reset", deltaG_reset->GetEndpoint (Endpoint::LHS));
    core_tsmgr->Connect ("Tsparity", core_TSParity->GetEndpoint (Endpoint::LHS));
    
    /*** Export port ***/    
    ExportPort ("Core_out", neuron_block, "NeuronBlock_out");
    ExportPort ("CurTSParity", core_tsmgr, "curTS");
    ExportPort ("DynFin", core_tsmgr, "DynFin");
}










