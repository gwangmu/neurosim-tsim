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
public:
    PCIeMessage (const char *clsname) : Message (clsname) {}

    PCIeMessage (const char *clsname, uint32_t payload_size, 
            uint32_t busid, uint32_t devid)
        : Message (clsname, -1, payload_size + PACKET_HEADER_SIZE), 
          BUS_ID (busid), DEV_ID (devid)
    {
        if (size == 0)
            DESIGN_FATAL ("PCIeMessage cannot be zero-sized",
                    "PCIeMessage");
    }

public:
    // NOTE: Start/Seq/Header + ECRC/LCRC/End = 26 Bytes
    const uint32_t PACKET_HEADER_SIZE = 26 * 8;

    const uint32_t BUS_ID;
    const uint32_t DEV_ID;
};
