#pragma once

#include <TSim/Pathway/Message.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: derive this class to make a PCIeMessage
struct PCIeMessage: public Message
{
public:
    PCIeMessage (const char *clsname) : Message (clsname) {}

    PCIeMessage (const char *clsname, uint32_t busid, uint32_t devid, uint32_t size)
        : Message (clsname, -1), BUS_ID (busid),
          DEV_ID (devid), SIZE (size)
    {
        if (unlikely(size == 0))
            DESIGN_FATAL ("PCIeMessage cannot be zero-sized",
                    "PCIeMessage");
    }

public:
    const uint32_t BUS_ID;
    const uint32_t DEV_ID;
    const uint32_t SIZE;
};
