#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Device/AndGate.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Simulation/Testbench.h>

#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Component/DataEndpt.h>

#include <Component/NeuroChip.h>
#include <Component/Propagator.h>
#include <Component/Controller.h>

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

USING_TESTBENCH;

NeuroSim::NeuroSim (string iname, Component *parent)
    : Component ("NeuroSim", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    /** Parameters **/
    const int num_boards = 1;
    const int num_chips = GET_PARAMETER (num_chips);
    const int num_propagators = GET_PARAMETER (num_propagators);
    
    const int num_cores = GET_PARAMETER (num_cores);

    const int axon_entry_queue_size = 64;

    /** Components **/
    std::vector<Component*> neurochips;
    for (int i=0; i<num_chips; i++)
        neurochips.push_back (new NeuroChip ("chip" + to_string(i), this, num_cores, num_propagators));
    
    std::vector<Component*> propagators;
    for (int i=0; i<num_propagators; i++)
        propagators.push_back(new Propagator ("propagator" + to_string(i), this));

    Controller *controller = new Controller ("controller", this, num_boards);

    /** Modules **/
    AndGate *idle_and = new AndGate ("idle_and", this, num_propagators);
    AndGate *dynfin_and = new AndGate ("dynfin_and", this, num_chips);

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    std::vector<RRFaninWire*> axon_data;
    for (int i=0; i<num_propagators; i++)
        axon_data.push_back (new RRFaninWire (this, conattr, Prototype<AxonMessage>::Get(), num_chips + 1));
    FanoutWire *cur_TSParity = new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), 
            num_cores*num_chips + num_propagators);
    
    std::vector<FanoutWire*> syn_data;
    std::vector<FanoutWire*> syn_parity;
    std::vector<FanoutWire*> syn_cidx;
    std::vector<Wire*> prop_idle;
    for (int i=0; i<num_propagators; i++)
    {
        syn_data.push_back (new FanoutWire (this, conattr, Prototype<SynapseMessage>::Get(), num_chips));
        syn_parity.push_back (new FanoutWire (this, conattr, Prototype<SignalMessage>::Get(), num_chips));
        syn_cidx.push_back (new FanoutWire (this, conattr, Prototype<SelectMessage>::Get(), num_chips));
        prop_idle.push_back (new Wire (this, conattr, Prototype<IntegerMessage>::Get()));
    }
    
    RRFaninWire *board_axon = new RRFaninWire (this, conattr, Prototype<AxonMessage>::Get(), num_propagators);
    RRFaninWire *board_id = new RRFaninWire (this, conattr, Prototype<SelectMessage>::Get(), num_propagators);
    Wire *prop_idle_and = new Wire (this, conattr, Prototype<IntegerMessage>::Get());

    std::vector<Wire*> chip_dynfin;
    for (int i=0; i<num_chips; i++)
        chip_dynfin.push_back (new Wire (this, conattr, Prototype<IntegerMessage>::Get()));

    Wire *dynfin = new Wire (this, conattr, Prototype<IntegerMessage>::Get()); 

    /** Connect **/
    for (int i=0; i<num_chips; i++)
    {
        for (int p=0; p<num_propagators; p++)
        {
            neurochips[i]->Connect ("Axon" + to_string(p), axon_data[p]->GetEndpoint (Endpoint::LHS, i));
            axon_data[p]->GetEndpoint (Endpoint::LHS, i)->SetCapacity(1);
        }

        for (int c=0; c<num_cores; c++)
        {
            neurochips[i]->Connect ("CurTSParity" + to_string(c), 
                    cur_TSParity->GetEndpoint (Endpoint::RHS, i*num_cores + c));
        }

        neurochips[i]->Connect ("DynFin", chip_dynfin[i]->GetEndpoint (Endpoint::LHS));
        dynfin_and->Connect ("input" + to_string(i), chip_dynfin[i]->GetEndpoint (Endpoint::RHS));
    }


    //ds_parity->Connect ("dataout", cur_TSParity->GetEndpoint (Endpoint::LHS));
    //ds_board->Connect ("datain", board_axon[i]->GetEndpoint (Endpoint::RHS));
    idle_and->Connect ("output", prop_idle_and->GetEndpoint (Endpoint::LHS));
    dynfin_and->Connect ("output", dynfin->GetEndpoint (Endpoint::LHS));

    controller->Connect ("TSParity", cur_TSParity->GetEndpoint (Endpoint::LHS));
    controller->Connect ("AxonIn", board_axon->GetEndpoint (Endpoint::RHS));
    controller->Connect ("BoardID", board_id->GetEndpoint (Endpoint::RHS));
    controller->Connect ("Idle", prop_idle_and->GetEndpoint (Endpoint::RHS));
    controller->Connect ("DynFin", dynfin->GetEndpoint (Endpoint::RHS));   

    //ds_idle->Connect ("datain", prop_idle[i]->GetEndpoint (Endpoint::RHS));
    for (int i=0; i<num_propagators; i++)
    {
        propagators[i]->Connect ("Axon", axon_data[i]->GetEndpoint (Endpoint::RHS));
        propagators[i]->Connect ("PropTS", cur_TSParity->GetEndpoint (Endpoint::RHS, num_chips*num_cores + i));
        
        propagators[i]->Connect ("Synapse", syn_data[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("SynTS", syn_parity[i]->GetEndpoint (Endpoint::LHS));
        propagators[i]->Connect ("Index", syn_cidx[i]->GetEndpoint (Endpoint::LHS));
        
        propagators[i]->Connect ("BoardAxon", board_axon->GetEndpoint (Endpoint::LHS, i));
        propagators[i]->Connect ("BoardID", board_id->GetEndpoint (Endpoint::LHS, i));
        board_axon->GetEndpoint (Endpoint::LHS, i)->SetCapacity (4);
        board_id->GetEndpoint (Endpoint::LHS, i)->SetCapacity (4);
        
        propagators[i]->Connect ("Idle", prop_idle[i]->GetEndpoint (Endpoint::LHS));
    
        syn_data[i]->GetEndpoint (Endpoint::LHS)->SetCapacity (axon_entry_queue_size);
        syn_parity[i]->GetEndpoint (Endpoint::LHS)->SetCapacity (axon_entry_queue_size);
        syn_cidx[i]->GetEndpoint (Endpoint::LHS)->SetCapacity (axon_entry_queue_size);

        idle_and->Connect ("input" + to_string(i), prop_idle[i]->GetEndpoint (Endpoint::RHS)); 
        controller->Connect ("AxonOut", axon_data[i]->GetEndpoint (Endpoint::LHS, num_chips));

        for (int c=0; c<num_chips; c++)
        {
            neurochips[c]->Connect ("SynapseData" + to_string(i), syn_data[i]->GetEndpoint (Endpoint::RHS, c));
            neurochips[c]->Connect ("SynTS" + to_string(i), syn_parity[i]->GetEndpoint (Endpoint::RHS, c));
            neurochips[c]->Connect ("SynCidx" + to_string(i), syn_cidx[i]->GetEndpoint (Endpoint::RHS, c));

        }
    }
}



