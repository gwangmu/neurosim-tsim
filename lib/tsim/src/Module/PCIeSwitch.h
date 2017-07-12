#include <TSim/Module/PCIeSwitch.h>
#include <TSim/Pathway/PCIeMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: GUESSING pipeline depth..
PCIeSwitch::PCIeSwitch (string iname, Component *parent,
        uint32_t n_ports, uint32_t inque_size, uint32_t outque_size,
        uint32_t busid)
    : Module ("PCIeSwitch", iname, parent, 10)
{
    this->n_ports = n_ports;
    if (n_ports == 0)
        DESIGN_FATAL ("PCIeSwitch must have at least one port",
                GetName().c_str());

    this->outque_size = outque_size;
    if (outque_size <= 1)
        DESIGN_FATAL ("PCIeSwitch must have OUTPUT queue",
                GetName().c_str());

    this->inque_size = inque_size;
    if (inque_size <= 1)
        DESIGN_FATAL ("PCIeSwitch must have INPUT queue",
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

    this->busid = busid;
    txsize = rxsize = 0;
    next_idx = cur_idx = -1;
}


bool PCIeSwitch::IsValidConnection (Port *port, Endpoint *endpt)
{
    Module::IsValidConnection (port, endpt);

    if (port->iotype == Unit::PORT_INPUT)
    {
        DESIGN_INFO ("resetting input queue size to %u..",
                GetName().c_str(), inque_size);
        endpt->SetCapacity (inque_size);
    }

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
    // NOTE: DEV_ID=-1 for broadcast 
    cur_idx = next_idx;

    operation ("route")
    {
        if (cur_idx != -1 && inmsgs[cur_idx])
        {
            PCIeMessage *pciemsg = static_cast<PCIeMessage *>(inmsgs[cur_idx]);

            if (unlikely (pciemsg->BUS_ID != busid))
                SIM_FATAL ("multi-hierarchy PCIe network not supported",
                        GetName().c_str());
            
            if (likely (pciemsg->DEV_ID < n_ports))
            {
                pciemsg->MarkRecycle ();
                outmsgs[pciemsg->DEV_ID] = pciemsg;
            }
            else
            {
                SIM_WARNING ("attemped to deliver PCIe msg to unknown destination %u",
                        GetName().c_str(), pciemsg->DEV_ID);
            }
        }
    }
    
    operation ("select")
    {
        bool selected = false;
        for (auto i = cur_idx + 1; i < n_ports; i++)
        {
            if (inmsgs[i])
            {
                selected = true;
                next_idx = i;
                break;
            }
        }

        if (!selected)
        {
            for (auto i = 0; i <= cur_idx; i++)
            {
                if (inmsgs[i])
                {
                    selected = true;
                    next_idx = i;
                    break;
                }
            }
        }

        if (!selected)
            next_idx = -1;
    }

    operation ("unpop inputs")
    {
        for (auto i = 0; i < n_ports; i++)
            if (i != cur_idx)
                inmsgs[i] = nullptr;
    }
}
