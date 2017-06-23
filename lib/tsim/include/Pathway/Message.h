#pragma once

#include <Base/Metadata.h>
#include <cinttypes>

using namespace std;

struct Message: public Metadata
{
public:
    Message (const char *clsname, string name, uint32_t destrhsid) 
        : Metadata (clsname, name), DEST_RHS_ID (destrhsid) {}

    const uint32_t DEST_RHS_ID;     // NOTE: -1 to broadcast
};
