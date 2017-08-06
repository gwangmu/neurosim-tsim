#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>
#include <TSim/Pathway/Link.h>
#include <TSim/Pathway/RemoteMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: this class is for enforcing Link frequency
namespace TSim
{
    class ICController: public Module
    {
    private:
        uint32_t PORT_TX;
        uint32_t PORT_TX_EXPORT;
        uint32_t PORT_RX;
        uint32_t PORT_RX_IMPORT;
    
    public:
        ICController (string iname, Component *parent, string clkname, 
                RemoteMessage *msgproto)
            : Module ("ICController", iname, parent, 1)
        {
            PORT_TX = CreatePort ("tx", Module::PORT_INPUT, msgproto);
            PORT_TX_EXPORT = CreatePort ("tx_export", Module::PORT_OUTPUT, msgproto);
            PORT_RX_IMPORT = CreatePort ("rx_import", Module::PORT_INPUT, msgproto);
            PORT_RX = CreatePort ("rx", Module::PORT_OUTPUT, msgproto);
    
            SetClock (clkname);
        }
    
        virtual bool IsValidConnection (Port *port, Endpoint *endpt)
        {
            Module::IsValidConnection (port, endpt);
    
            if ((port->name == "tx_export" || port->name == "rx_import") &&
                    !dynamic_cast<Link *>(endpt->GetParent()))
            {
                DESIGN_FATAL ("importing/exporting port must be connected with Link",
                        GetName().c_str());
                return false;
            }
    
            return true;
        }
    
        virtual void Operation (Message **inmsgs, Message **outmsgs, 
                Instruction *instr)
        {
            // NOTE: simply forwarding messages
            if (inmsgs[PORT_TX])
            {
                inmsgs[PORT_TX]->Recycle();
                outmsgs[PORT_TX_EXPORT] = inmsgs[PORT_TX];
            }
    
            if (inmsgs[PORT_RX_IMPORT])
            {
                inmsgs[PORT_RX_IMPORT]->Recycle();
                outmsgs[PORT_RX] = inmsgs[PORT_RX_IMPORT];
            }
        }
    };
}
