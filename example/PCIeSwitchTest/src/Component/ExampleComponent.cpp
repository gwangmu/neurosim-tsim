#include <TSim/Pathway/Link.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/PCIeSwitch.h>

#include <Message/ExamplePCIeMessage.h>
#include <Component/PCIeTxComponent.h>
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
    vector<Component *> sources;
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
        sources.push_back(new PCIeTxComponent ("txcomp" + to_string(i), this, i));

    PCIeSwitch *pswitch = new PCIeSwitch ("pswitch", this, "pcie",
            Prototype<ExamplePCIeMessage>::Get(), 4, 4, 4);

    // create/connect pathways
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
    {
        Pathway::ConnectionAttr conattr (0, 12);    // NOTE: x12 lane

        Link *m2s_wire = new Link (this, conattr, 5, Prototype<ExamplePCIeMessage>::Get());
        sources[i]->Connect ("tx_export", m2s_wire->GetEndpoint (Endpoint::LHS));
        pswitch->Connect ("rx" + to_string(i), m2s_wire->GetEndpoint (Endpoint::RHS));

        Link *s2m_wire = new Link (this, conattr, 5, Prototype<ExamplePCIeMessage>::Get());
        pswitch->Connect ("tx" + to_string(i), s2m_wire->GetEndpoint (Endpoint::LHS));
        sources[i]->Connect ("rx_import", s2m_wire->GetEndpoint (Endpoint::RHS));
    }
}
