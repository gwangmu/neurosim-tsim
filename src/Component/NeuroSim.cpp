#include <Component/NeuroSim.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Utility/Prototype.h>

#include <Component/NeuronBlock.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Message/ExampleMessage.h>
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
    Module *datasink = new DataSinkModule ("datasink", this);

    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    Wire *src2nb = new Wire (this, conattr, Prototype<NeuronBlockInMessage>::Get());
    Wire *nb2sink = new Wire (this, conattr, Prototype<NeuronBlockOutMessage>::Get());

    src2nb->GetEndpoint (Endpoint::LHS)->SetCapacity (0);
    nb2sink->GetEndpoint (Endpoint::LHS)->SetCapacity (0);

    // connect modules
    datasource->Connect ("dataout", src2nb->GetEndpoint (Endpoint::LHS));
    neuron_block->Connect ("NeuronBlock_in", src2nb->GetEndpoint (Endpoint::RHS));

    neuron_block->Connect ("NeuronBlock_out", nb2sink->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", nb2sink->GetEndpoint (Endpoint::RHS));
}
