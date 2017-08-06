#include <TSim/Pathway/Link.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/RouterNetwork.h>

#include <Message/ExampleRemoteMessage.h>
#include <Component/DataSourceModule.h>
#include <Component/ExampleComponent.h>
#include <Component/DataSourceModule.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <cmath>

using namespace std;
using namespace TSim;


ExampleComponent::ExampleComponent (string iname, Component *parent)
    : Component ("ExampleComponent", iname, parent)
{
    SetClock ("main");

    // add child modules/components
    vector<Component *> sources;
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
        sources.push_back(new DataSourceModule ("txcomp" + to_string(i), this, "pcie", i));

    RouterNetwork *routernet = new RouterNetwork ("routernet", this, "pcie",
            Prototype<ExampleRemoteMessage>::Get(), "torus", 512, MAX_TEST_SOURCES);

    // create/connect pathways
    for (auto i = 0; i < MAX_TEST_SOURCES; i++)
    {
        Pathway::ConnectionAttr conattr (0, 12);    // NOTE: x12 lane

        Link *m2s_wire = new Link (this, conattr, 5, Prototype<ExampleRemoteMessage>::Get());
        sources[i]->Connect ("out", m2s_wire->GetEndpoint (Endpoint::LHS));
        routernet->Connect ("rx" + to_string(i), m2s_wire->GetEndpoint (Endpoint::RHS));

        Link *s2m_wire = new Link (this, conattr, 5, Prototype<ExampleRemoteMessage>::Get());
        routernet->Connect ("tx" + to_string(i), s2m_wire->GetEndpoint (Endpoint::LHS));
        sources[i]->Connect ("in", s2m_wire->GetEndpoint (Endpoint::RHS));
    }
}
