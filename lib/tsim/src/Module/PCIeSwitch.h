#include <TSim/Module/PCIeSwitch.h>
#include <TSim/Pathway/PCIeMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

PCIeSwitch::PCIeSwitch (string iname, Component *parent, uint32_t n_ports, 
        uint32_t outque_size, uint32_t bytewidth)
    : Module ("PCIeSwitch", iname, parent, 1)
{
    this->n_ports = n_ports;
    if (n_ports == 0)
        DESIGN_FATAL ("PCIeSwitch must have at least one port",
                GetName().c_str());

    this->outque_size = outque_size;
    if (outque_size <= 1)
        DESIGN_FATAL ("PCIeSwitch must have queued OUTPUT",
                GetName().c_str());

    PORT_RX = new uint32_t[n_ports] ();
    PORT_TX = new uint32_t[n_ports] ();

    for (auto i = 0; i < n_ports; i++)
    {
        PORT_RX[i] = CREATE_PORT ("rx" + to_string(i), Module::PORT_INPUT,
                Prototype<PCIeMessage>::Get());
        PORT_TX[i] = CREATE_PORT ("tx" + to_string(i), Module::PORT_OUTPUT,
                Prototype<PCIeMessage>::Get());
    }

    txsize = rxsize = 0;
}


bool PCIeSwitch::IsValidConnection (Port *port, Endpoint *endpt)
{
    Module::IsValidConnection (port, endpt);

    if (port->iotype == Unit::PORT_OUTPUT)
    {
        DESIGN_INFO ("resetting output queue size to %u..",
                GetName().c_str(), outque_size);
        endpt->SetCapacity (outque_size);
    }

    return true;
}


void PCIeSwitch::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    
}
