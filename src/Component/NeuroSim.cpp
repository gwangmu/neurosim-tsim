#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/NeuronBlock.h>
#include <Component/NBController.h>
#include <Component/SRAMModule.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>

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
    //Module *datasource = new DataSourceModule ("datasource", this);
   
    Module *neuron_block = new NeuronBlock ("neuron_block", this, 2);
   
    //Module *state_sram = new SRAMModule <StateMessage, State> ("state_sram", this, 64, 16);
    //Module *deltaG_sram = new SRAMModule <DeltaGMessage, uint64_t> ("deltaG_sram", this, 64, 16);

    Module *datasink = new DataSinkModule ("datasink", this);
    Module *nb_controller = new NBController ("nb_controller", this, 16);

    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    
    // Neuron Block
    Wire *ctrl2nb = new Wire (this, conattr, Prototype<NeuronBlockInMessage>::Get());
    Wire *nb2sink = new Wire (this, conattr, Prototype<NeuronBlockOutMessage>::Get());

    // Neuron Block Controller
    FanoutWire *core_sram_raddr = new FanoutWire (this, conattr, Prototype<IndexMessage>::Get(), 2);

    Wire *state_sram_rdata = new Wire (this, conattr, Prototype<StateMessage>::Get());
    Wire *deltaG_sram_rdata = new Wire (this, conattr, Prototype<DeltaGMessage>::Get());


    // ctrl2nb->GetEndpoint (Endpoint::LHS)->SetCapacity (0);
    // nb2sink->GetEndpoint (Endpoint::LHS)->SetCapacity (0);

    // core_sram_raddr->GetEndpoint (Endpoint::LHS)->SetCapacity (0);
    // state_sram_rdata->GetEndpoint (Endpoint::LHS)->SetCapacity (0);
    // deltaG_sram_rdata->GetEndpoint (Endpoint::LHS)->SetCapacity (0);
    
    /*** Connect modules ***/
    // Neuron block controller
    //nb_controller->Connect ("nb", ctrl2nb->GetEndpoint (Endpoint::LHS));
    //nb_controller->Connect ("sram", core_sram_raddr->GetEndpoint (Endpoint::LHS));
    nb_controller->Connect ("nb", Endpoint::PORTCAP());
    nb_controller->Connect ("sram", Endpoint::PORTCAP());
    nb_controller->Connect ("end", Endpoint::PORTCAP());

    //nb_controller->Connect ("deltaG", deltaG_sram_rdata->GetEndpoint (Endpoint::RHS));
    //nb_controller->Connect ("state", state_sram_rdata->GetEndpoint (Endpoint::RHS));
    nb_controller->Connect ("deltaG", Endpoint::PORTCAP());
    nb_controller->Connect ("state", Endpoint::PORTCAP());
    nb_controller->Connect ("tsparity", Endpoint::PORTCAP());
        
    // State & Delta-G SRAM
    // state_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS)); 
    // state_sram->Connect ("r_data", state_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    // state_sram->Connect ("w_addr", Endpoint::PORTCAP());
    // state_sram->Connect ("w_data", Endpoint::PORTCAP());
    // 
    // deltaG_sram->Connect ("r_addr", core_sram_raddr->GetEndpoint (Endpoint::RHS)); 
    // deltaG_sram->Connect ("r_data", deltaG_sram_rdata->GetEndpoint (Endpoint::LHS)); 
    // deltaG_sram->Connect ("w_addr", Endpoint::PORTCAP());
    // deltaG_sram->Connect ("w_data", Endpoint::PORTCAP());
    
    // Neuron block
    neuron_block->Connect ("NeuronBlock_in", ctrl2nb->GetEndpoint (Endpoint::RHS));
    neuron_block->Connect ("NeuronBlock_out", nb2sink->GetEndpoint (Endpoint::LHS));

    datasink->Connect ("datain", nb2sink->GetEndpoint (Endpoint::RHS));
}






















