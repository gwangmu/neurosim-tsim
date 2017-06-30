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

    // add child modules/components
    Module *datasource = new DataSourceModule ("datasource", this);
    Module *neuron_block = new NeuronBlock ("neuron_block", this, 2);
   
    Module *state_sram = new StateSRAM ("state_sram", this, 64, 16);
    Module *deltaG_sram = new DeltaSRAM ("deltaG_sram", this, 64, 16);

    Module *datasink = new DataSinkModule <NeuronBlockOutMessage, uint32_t> ("datasink", this);
    Module *nb_controller = new NBController ("nb_controller", this, 16);
   
    
    // Dummy modules
    Module *ds_end = new DataSinkModule <SignalMessage, bool> ("ds_end", this);
    Module *ds_idle = new DataSinkModule <SignalMessage, bool> ("ds_idle", this);

    Module *de_d_waddr = new DataEndptModule <IndexMessage> ("de_d_waddr", this);
    Module *de_d_wdata = new DataEndptModule <DeltaGMessage> ("de_d_wdata", this);
    
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Neuron Block
    Wire *ctrl2nb = new Wire (this, conattr, Prototype<NeuronBlockInMessage>::Get());
    Wire *nb2sink = new Wire (this, conattr, Prototype<NeuronBlockOutMessage>::Get());

    Wire *state_waddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *state_wdata = new Wire (this, conattr, Prototype<StateMessage>::Get());

    // Neuron Block Controller
    FanoutWire *core_sram_raddr = new FanoutWire (this, conattr, Prototype<IndexMessage>::Get(), 2);

    Wire *state_sram_rdata = new Wire (this, conattr, Prototype<StateMessage>::Get());
    Wire *deltaG_sram_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
    Wire *core_ts_parity = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    // ctrl2nb->GetEndpoint (Endpoint::LHS)->SetCapacity (0);

    
    // Dummy wire
    Wire *ds_end_wire = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *ds_idle_wire = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *de_d_waddr_wire = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *de_d_wdata_wire = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());
    
    // Dummy Connection
    ds_end->Connect ("datain", ds_end_wire->GetEndpoint (Endpoint::RHS));
    ds_idle->Connect ("datain", ds_idle_wire->GetEndpoint (Endpoint::RHS));
    de_d_waddr->Connect ("dataend", de_d_waddr_wire->GetEndpoint (Endpoint::LHS));
    de_d_wdata->Connect ("dataend", de_d_wdata_wire->GetEndpoint (Endpoint::LHS));

    /*** Connect modules ***/
    // Neuron block controller
    nb_controller->Connect ("neuron_block", ctrl2nb->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("sram", core_sram_raddr->GetEndpoint (Endpoint::LHS));
    //nb_controller->Connect ("end", Endpoint::PORTCAP());
    nb_controller->Connect ("end", ds_end_wire->GetEndpoint (Endpoint::LHS));

    nb_controller->Connect ("deltaG", deltaG_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("state", state_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("tsparity", core_ts_parity->GetEndpoint (Endpoint::RHS));
    //nb_controller->Connect ("tsparity", Endpoint::PORTCAP());
        
    // State & Delta-G SRAM
    state_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS, 0)); 
    state_sram->Connect ("r_data", state_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    state_sram->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::RHS));
    state_sram->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::RHS));

    deltaG_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS, 1)); 
    deltaG_sram->Connect ("r_data", deltaG_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    //deltaG_sram->Connect ("w_addr", Endpoint::PORTCAP());
    //deltaG_sram->Connect ("w_data", Endpoint::PORTCAP());
    deltaG_sram->Connect ("w_addr", de_d_waddr_wire->GetEndpoint (Endpoint::RHS));
    deltaG_sram->Connect ("w_data", de_d_wdata_wire->GetEndpoint (Endpoint::RHS));
    
    // Neuron block
    neuron_block->Connect ("NeuronBlock_in", ctrl2nb->GetEndpoint (Endpoint::RHS));
    neuron_block->Connect ("NeuronBlock_out", nb2sink->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("w_addr", state_waddr->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("w_data", state_wdata->GetEndpoint (Endpoint::LHS));
    //neuron_block->Connect ("w_idle", Endpoint::PORTCAP());
    neuron_block->Connect ("idle", ds_idle_wire->GetEndpoint (Endpoint::LHS));

    datasink->Connect ("datain", nb2sink->GetEndpoint (Endpoint::RHS));
    datasource->Connect ("dataout", core_ts_parity->GetEndpoint (Endpoint::LHS));
}






















