#pragma once

#include <TSim/Pathway/RemoteMessage.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: derive this class to make a PCIeMessage
namespace TSim
{
    struct PCIeMessage: public RemoteMessage
    {
    private:
        // NOTE: Start/Seq/Header + ECRC/LCRC/End = 26 Bytes
        static const uint32_t PACKET_HEADER_SIZE = 26 * 8;
    
    public:
        PCIeMessage (const char *clsname) 
            : RemoteMessage (clsname) {}
    
        PCIeMessage (const char *clsname, uint32_t payload_size, 
                uint32_t dev_id, uint32_t bus_id = 0)
            : RemoteMessage (clsname, payload_size + PACKET_HEADER_SIZE,
                    dev_id, bus_id)
        {
            if (payload_size == 0)
                DESIGN_FATAL ("PCIeMessage cannot be zero-sized",
                        "PCIeMessage");
        }
    
        virtual PCIeMessage* Clone () = 0;
    
    public:
        const uint32_t& DEV_ID = DST_ID0;
        const uint32_t& BUS_ID = DST_ID1;
    };
}
