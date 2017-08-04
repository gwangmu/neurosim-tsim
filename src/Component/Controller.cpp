#include <Component/Controller.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Module/PCIeController.h>

#include <Component/TSManager.h>
#include <Component/PacketConstructor.h>
#include <Component/PacketDecoder.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

Controller::Controller (string iname, Component *parent, uint32_t num_board)
    : Component ("Controller", iname, parent)
{
    /** Parameters **/
    int num_propagators = GET_PARAMETER(num_propagators);

    /** Components **/
   
    /** Modules **/
    Module *ts_mgr = new TSManager ("ts_manager", this, num_board);
    Module *pkt_constructor = new PacketConstructor ("packet_constructor", this);
    Module *pkt_decoder = new PacketDecoder ("packet_decoder", this);

    Module *pcie_controller = new PCIeController ("pcie_controller", this,
            "pcie", Prototype<PacketMessage>::Get());

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    Wire *board_tsend = new Wire (this, conattr, Prototype<SignalMessage>::Get());
    Wire *remote_tsend = new Wire (this, conattr, Prototype<SignalMessage>::Get());

    Wire *pkt_send = new Wire (this, conattr, Prototype<PacketMessage>::Get());
    Wire *pkt_recv = new Wire (this, conattr, Prototype<PacketMessage>::Get());

    /** Connect **/
    ts_mgr->Connect ("tsend", board_tsend->GetEndpoint (Endpoint::LHS));
    pkt_constructor->Connect ("ts_end", board_tsend->GetEndpoint (Endpoint::RHS));

    pkt_constructor->Connect ("packet", pkt_send->GetEndpoint (Endpoint::LHS));
    pcie_controller->Connect ("tx", pkt_send->GetEndpoint (Endpoint::RHS));

    pcie_controller->Connect ("rx", pkt_recv->GetEndpoint (Endpoint::LHS));
    pkt_decoder->Connect ("packet", pkt_recv->GetEndpoint (Endpoint::RHS));
    
    pkt_decoder->Connect ("ts_end", remote_tsend->GetEndpoint (Endpoint::LHS));
    ts_mgr->Connect ("remote_tsend", remote_tsend->GetEndpoint (Endpoint::RHS));

    ExportPort ("DynFin", ts_mgr, "dynfin");
    ExportPort ("Idle", ts_mgr, "idle");
    ExportPort ("TSParity", ts_mgr, "ts_parity");
    
    ExportPort ("AxonIn", pkt_constructor, "axon"); 
    ExportPort ("BoardID", pkt_constructor, "board_id");

    for (int i=0; i<num_propagators; i++)
    {
        ExportPort ("AxonOut" + to_string(i), 
                    pkt_decoder, "axon" + to_string(i));
        ExportPort ("Bypass" + to_string(i), 
                    pkt_decoder, "bypass" + to_string(i));
    }

    // gwangmu
    ExportPort ("PCIeTxExport", pcie_controller, "tx_export");
    ExportPort ("PCIeRxImport", pcie_controller, "rx_import");
}



