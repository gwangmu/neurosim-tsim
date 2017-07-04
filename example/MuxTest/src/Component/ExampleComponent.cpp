#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Device/Mux.h>

#include <Component/ExampleComponent.h>
#include <Component/DataSourceModule.h>
#include <Component/DataSinkModule.h>
#include <Message/ExampleMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleComponent::ExampleComponent (string iname, Component *parent)
    : Component ("ExampleComponent", iname, parent)
{
    // NOTE: children automatically inherit parent's clock
    //  but they can override it by redefining their own.
    SetClock ("main");

    // add child modules/components
    Module *datasource = new DataSourceModule ("datasource", this);
    Module *datasource2 = new DataSourceModule ("datasource2", this);
    Module *datasink = new DataSinkModule ("datasink", this);

    Mux *mux = new Mux ("mux", this, Prototype<ExampleMessage>::Get(), 2);

    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    Wire *d2mwire = new Wire (this, conattr, Prototype<ExampleMessage>::Get());
    Wire *d2mwire2 = new Wire (this, conattr, Prototype<ExampleMessage>::Get());
    Wire *postmux_wire = new Wire (this, conattr, Prototype<ExampleMessage>::Get());
    Wire *ctrl_wire = new Wire (this, conattr, Prototype<IntegerMessage>::Get());

    // connect modules
    datasource->Connect ("dataout", d2mwire->GetEndpoint (Endpoint::LHS));
    mux->Connect ("input0", d2mwire->GetEndpoint (Endpoint::RHS));

    datasource2->Connect ("dataout", d2mwire2->GetEndpoint (Endpoint::LHS));
    mux->Connect ("input1", d2mwire2->GetEndpoint (Endpoint::RHS));

    mux->Connect ("output", postmux_wire->GetEndpoint (Endpoint::LHS));
    datasink->Connect ("datain", postmux_wire->GetEndpoint (Endpoint::RHS));

    datasink->Connect ("select_mux", ctrl_wire->GetEndpoint (Endpoint::LHS));
    mux->Connect ("select", ctrl_wire->GetEndpoint (Endpoint::RHS));
}
