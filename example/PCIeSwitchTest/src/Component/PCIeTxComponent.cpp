#include <TSim/Pathway/Wire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/PCIeController.h>

#include <Message/ExamplePCIeMessage.h>
#include <Component/PCIeTxComponent.h>
#include <Component/DataSourceModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


PCIeTxComponent::PCIeTxComponent (string iname, Component *parent, uint32_t seedid)
    : Component ("PCIeTxComponent", iname, parent)
{
    SetClock ("main");

    Module *srcmod = new DataSourceModule ("datasource" + to_string(seedid), this, seedid);
    PCIeController *pctrl = new PCIeController ("pctrl", this, "pcie",
            Prototype<ExamplePCIeMessage>::Get());

    Pathway::ConnectionAttr conattr (0, 64);
    Wire *s2c_wire = new Wire (this, conattr, Prototype<ExamplePCIeMessage>::Get());
    srcmod->Connect ("out", s2c_wire->GetEndpoint (Endpoint::LHS));
    pctrl->Connect ("tx", s2c_wire->GetEndpoint (Endpoint::RHS));

    Wire *c2s_wire = new Wire (this, conattr, Prototype<ExamplePCIeMessage>::Get());
    pctrl->Connect ("rx", c2s_wire->GetEndpoint (Endpoint::LHS));
    srcmod->Connect ("in", c2s_wire->GetEndpoint (Endpoint::RHS));

    ExportPort ("tx_export", pctrl, "tx_export");
    ExportPort ("rx_import", pctrl, "rx_import");
}
