#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>

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

    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);
    RRFaninWire *d2dwire = new RRFaninWire (this, conattr, Prototype<ExampleMessage>::Get(), 2);

    // connect modules
    datasource->Connect ("dataout", d2dwire->GetEndpoint (Endpoint::LHS, 0));
    datasource2->Connect ("dataout", d2dwire->GetEndpoint (Endpoint::LHS, 1));
    datasink->Connect ("datain", d2dwire->GetEndpoint (Endpoint::RHS));
}
