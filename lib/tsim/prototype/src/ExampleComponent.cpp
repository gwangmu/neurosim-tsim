/***
 * TSim Component prototype (body)
 **/

#include <Prototype/ExampleComponent.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleModule::ExampleModule (string iname, Component *parent)
{
    #if 0
    // (TODO) add child modules/components
    ExampleModule *dummymodule1 = new ExampleModule ("dummy", this);
    AddChild (dummymodule1);

    ExampleModule *dummymodule2 = new ExampleModule ("dummy", this);
    AddChild (dummymodule2);

    // (TODO) create pathways
    Pathway::ConnectionAttr conattr;
    conattr.latency = 0;
    conattr.bitwidth = 32;
    Wire *d2dwire = new Wire (this, conattr, new SpikeMessage ());

    // (TODO) connect modules
    dummymodule1.Connect ("spikeout", d2dwire.GetEndpoint (Endpoint::LHS));
    dummymodule2.Connect ("spikein", d2dwire.GetEndpoint (Endpoint::RHS));
    #endif
}
