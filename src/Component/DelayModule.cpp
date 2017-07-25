#include <Component/DelayModule.h>

#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/RRFaninWire.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Component/DelayAxonMgr.h>
#include <Component/DelaySRAM.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/DelayMetaMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

USING_TESTBENCH;

DelayModule::DelayModule (string iname, Component *parent)
    : Component ("DelayModule", iname, parent)
{
    /** Parameters **/
    int delay_storage_size = 2048;

    /** Components **/
   
    /** Modules **/
    Module *delay_mgr = new DelayAxonMgr ("delay_axon_mgr", this); 
    Module *delay_storage = new DelaySRAM ("delay_storage", this,
                                           128, delay_storage_size);

    /** Module & Wires **/
    // create pathways
    Pathway::ConnectionAttr conattr (0, 32);

    // Wires
    Wire *raddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *waddr = new Wire (this, conattr, Prototype<IndexMessage>::Get());
    Wire *rdata = new Wire (this, conattr,
                            Prototype<DelayMetaMessage>::Get());
    Wire *wdata = new Wire (this, conattr,
                            Prototype<DelayMetaMessage>::Get());

    /** Connect **/
    delay_mgr->Connect ("raddr", raddr->GetEndpoint (Endpoint::LHS));
    delay_mgr->Connect ("waddr", waddr->GetEndpoint (Endpoint::LHS));
    delay_mgr->Connect ("rdata", rdata->GetEndpoint (Endpoint::RHS));
    delay_mgr->Connect ("wdata", wdata->GetEndpoint (Endpoint::LHS));
   
    delay_storage->Connect ("raddr", raddr->GetEndpoint (Endpoint::RHS));
    delay_storage->Connect ("waddr", waddr->GetEndpoint (Endpoint::RHS));
    delay_storage->Connect ("rdata", rdata->GetEndpoint (Endpoint::LHS));
    delay_storage->Connect ("wdata", wdata->GetEndpoint (Endpoint::RHS));

    ExportPort ("Input", delay_mgr, "delay_input");
    ExportPort ("Output", delay_mgr, "axon_output");
    ExportPort ("TSParity", delay_mgr, "curTS");
    ExportPort ("Idle", delay_mgr, "idle");
}



