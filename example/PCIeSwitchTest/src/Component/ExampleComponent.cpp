#include <TSim/Pathway/Wire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/PCIeSwitch.h>

#include <Message/ExamplePCIeMessage.h>
#include <Component/ExampleComponent.h>
#include <Component/DataSourceModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


ExampleComponent::ExampleComponent (string iname, Component *parent)
    : Component ("ExampleComponent", iname, parent)
{
    SetClock ("main");

    // add child modules/components
    vector<Module *> sources;
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
        sources.push_back(new DataSourceModule ("datasource" + to_string(i), this, i));

    PCIeSwitch *pswitch = new PCIeSwitch ("pswitch", this, 
            Prototype<ExamplePCIeMessage>::Get(), 4, 4, 4);

    // create/connect pathways
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
    {
        Pathway::ConnectionAttr conattr (0, 16);        // NOTE: x16 lanes

        Wire *m2s_wire = new Wire (this, conattr, Prototype<ExamplePCIeMessage>::Get());
        sources[i]->Connect ("out", m2s_wire->GetEndpoint (Endpoint::LHS));
        pswitch->Connect ("rx" + to_string(i), m2s_wire->GetEndpoint (Endpoint::RHS));

        Wire *s2m_wire = new Wire (this, conattr, Prototype<ExamplePCIeMessage>::Get());
        pswitch->Connect ("tx" + to_string(i), s2m_wire->GetEndpoint (Endpoint::LHS));
        sources[i]->Connect ("in", s2m_wire->GetEndpoint (Endpoint::RHS));
    }
}
