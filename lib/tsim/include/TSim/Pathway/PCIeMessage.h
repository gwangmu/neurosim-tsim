#pragma once

#include <TSim/Pathway/Message.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: derive this class to make a PCIeMessage
struct PCIeMessage: public Message
{
private:
    // NOTE: Start/Seq/Header + ECRC/LCRC/End = 26 Bytes
    static const uint32_t PACKET_HEADER_SIZE = 26 * 8;

public:
    PCIeMessage (const char *clsname) 
        : Message (clsname), BUS_ID (-1), DEV_ID (-1) {}

    PCIeMessage (const char *clsname, uint32_t payload_size, 
            uint32_t devid, uint32_t busid = 0)
        : Message (clsname, -1, payload_size * 8 + PACKET_HEADER_SIZE), 
          BUS_ID (busid), DEV_ID (devid)
    {
        if (payload_size == 0)
            DESIGN_FATAL ("PCIeMessage cannot be zero-sized",
                    "PCIeMessage");
    }

    virtual PCIeMessage* Clone () = 0;

public:
    const uint32_t BUS_ID;
    const uint32_t DEV_ID;
};
