#pragma once

#include <TSim/Base/Metadata.h>
#include <cinttypes>

using namespace std;

struct Message: public Metadata
{
public:
    // For prototype creation. 
    // DO NOT USE FOR ORDINARY MESSAGE CREATION
    Message (const char *clsname)
        : Metadata (clsname, ""), DEST_RHS_ID (-1) {}

    Message (const char *clsname, uint32_t destrhsid)
        : Metadata (clsname, ""), DEST_RHS_ID (destrhsid) {}

    const uint32_t DEST_RHS_ID;     // NOTE: -1 to broadcast

/*>> Below this is for MESSAGE DISPOSAL <<*/
public:
    void SetNumDestination (uint32_t n_dest, PERMIT(Pathway)) { dispcount = n_dest; }
    void Dispose (PERMIT(Module)) 
    {
        dispcount--;
        if (dispcount == 0)
            delete this;        // COMMIT SUICIDE!
    }

private:
    uint32_t dispcount;
};
