#include <TSim/Module/PCIeSwitch.h>
#include <TSim/Pathway/PCIeMessage.h>
#include <TSim/Pathway/Link.h>
#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: GUESSING pipeline depth..
PCIeSwitch::PCIeSwitch (string iname, Component *parent, string clkname, 
        PCIeMessage *msgproto, uint32_t n_ports, uint32_t inque_size, 
        uint32_t outque_size, uint32_t busid)
    : Module ("PCIeSwitch", iname, parent, 10)
{
    SetClock (clkname);

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
        PORT_RX[i] = CreatePort ("rx" + to_string(i), Module::PORT_INPUT, msgproto);
        PORT_TX[i] = CreatePort ("tx" + to_string(i), Module::PORT_OUTPUT, msgproto);
    }

    this->busid = busid;
    traffic_size_bits = 0;
    next_idx = cur_idx = -1;
}


bool PCIeSwitch::IsValidConnection (Port *port, Endpoint *endpt)
{
    Module::IsValidConnection (port, endpt);

    if (endpt->GetEndpointType() != Endpoint::CAP)
    {
        if (!dynamic_cast<Link *>(endpt->GetParent()))
            DESIGN_FATAL ("must be connected with Link", GetName().c_str());

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
    }

    return true;
}


void PCIeSwitch::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // NOTE: DEV_ID=-1 for broadcast 
    cur_idx = next_idx;

    operation ("route")
    {
        if (cur_idx != -1 && inmsgs[PORT_RX[cur_idx]])
        {
            PCIeMessage *pciemsg = static_cast<PCIeMessage *>(inmsgs[PORT_RX[cur_idx]]);

            DEBUG_PRINT ("%p", pciemsg);
            if (unlikely (pciemsg->BUS_ID != busid))
                SIM_FATAL ("multi-hierarchy PCIe network not supported",
                        GetName().c_str());
            
            if (likely (pciemsg->DEV_ID < n_ports))
            {
                DEBUG_PRINT ("forwarded %p to %u (cur_idx: %u)", 
                        GetName().c_str(), pciemsg, pciemsg->DEV_ID, cur_idx);
                
                pciemsg->Recycle ();
                outmsgs[PORT_TX[pciemsg->DEV_ID]] = pciemsg;

                traffic_size_bits += pciemsg->BIT_WIDTH;
            }
            else if (pciemsg->DEV_ID == -1)
            {
                DEBUG_PRINT ("forwarded %p to everyone (cur_idx: %u)", 
                        GetName().c_str(), pciemsg, cur_idx);
                
                for (auto i = 0; i < n_ports; i++)
                {
                    if (i == cur_idx) continue; 
                    PCIeMessage* msgclone = pciemsg->Clone ();
                    msgclone->Recycle ();
                    outmsgs[PORT_TX[i]] = msgclone;
                }

                traffic_size_bits += pciemsg->BIT_WIDTH;
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
        next_idx = -1;
        for (auto i = cur_idx + 1; i < n_ports; i++)
        {
            if (inmsgs[PORT_RX[i]])
            {
                next_idx = i;
                break;
            }
        }

        if (cur_idx != -1 && next_idx == -1)
        {
            for (auto i = 0; i <= cur_idx; i++)
            {
                if (inmsgs[PORT_RX[i]])
                {
                    next_idx = i;
                    break;
                }
            }
        }
    }

    if (next_idx != -1)
    {
        operation ("unpop inputs")
        {
            for (auto i = 0; i < n_ports; i++)
                if (i != cur_idx)
                    inmsgs[PORT_RX[i]] = nullptr;
        }
    }
}
