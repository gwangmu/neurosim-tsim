#include <Component/NeuroSimSystem.h>

#include <TSim/Pathway/Link.h>
#include <TSim/Simulation/Testbench.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/PCIeSwitch.h>

#include <Component/NeuroSim.h>
#include <Component/TimestepReporter.h>
#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

NeuroSimSystem::NeuroSimSystem (string iname, Component *parent)
    : Component ("NeuroSimSystem", iname, parent)
{
    SetClock ("main");

    /** Parameters **/
    const int num_boards = GET_PARAMETER (num_boards);

    /** Components **/
    vector<Component *> neurosims;
    for (uint32_t i = 0; i < num_boards; i++)
        neurosims.push_back (new NeuroSim ("neurosim" + to_string(i), this, i));

    // NOTE: this TimestepReporter is only for simulation progress checking.
    tsrep = new TimestepReporter ("tsrep", this, num_boards);

    Component *pswitch = new PCIeSwitch ("pswitch", this, "pcie",
            Prototype<PacketMessage>::Get(), num_boards + 1, 64, 64);

    /** Pathways & Connections **/
    Pathway::ConnectionAttr conattr (0, 12);
    
    for (uint32_t i = 0; i < num_boards; i++)
    {
        Link *m2s_link = new Link (this, conattr, 5, 
                Prototype<PacketMessage>::Get());
        neurosims[i]->Connect ("PCIeTxExport", 
                m2s_link->GetEndpoint (Endpoint::LHS));
        pswitch->Connect ("rx" + to_string(i), 
                m2s_link->GetEndpoint (Endpoint::RHS));

        Link *s2m_link = new Link (this, conattr, 5, 
                Prototype<PacketMessage>::Get());
        pswitch->Connect ("tx" + to_string(i), 
                s2m_link->GetEndpoint (Endpoint::LHS));
        neurosims[i]->Connect ("PCIeRxImport", 
                s2m_link->GetEndpoint (Endpoint::RHS));
    }

    Link *s2m_link = new Link (this, conattr, 5, Prototype<PacketMessage>::Get());
    pswitch->Connect ("tx" + to_string(num_boards), 
            s2m_link->GetEndpoint (Endpoint::LHS));
    tsrep->Connect ("remote_tsend", s2m_link->GetEndpoint (Endpoint::RHS));
    
    pswitch->Connect ("rx" + to_string(num_boards), Endpoint::PORTCAP());
}
