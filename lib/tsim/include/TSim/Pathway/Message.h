#pragma once

#include <TSim/Base/Metadata.h>
#include <cinttypes>

using namespace std;

struct Message: public Metadata
{
public:
    enum Type { PLAIN, TOGGLE };

    // For prototype creation. 
    // DO NOT USE FOR ORDINARY MESSAGE CREATION
    Message (const char *clsname)
        : Metadata (clsname, ""), DEST_RHS_ID (-1), type (PLAIN) {}

    Message (const char *clsname, uint32_t destrhsid)
        : Metadata (clsname, ""), DEST_RHS_ID (destrhsid), type (PLAIN) {}

    Message (const char *clsname, Type type, uint32_t destrhsid)
        : Metadata (clsname, ""), DEST_RHS_ID (destrhsid), type (type) {}

    uint32_t DEST_RHS_ID;     // NOTE: -1 to broadcast

/*>> Below this is for OUTPUT RESERVATION <<*/
public:
    // NOTE: reserve message has signature '10' in its 63rd~62nd bits 
    static inline Message* RESERVE ()
    {
        return (Message *)((uint64_t)1 << 63); 
    }

    static inline uint32_t IsReserveMsg (Message *msg)
    {
        return ((uint64_t)msg >> 62 == 0x2);
    }

/*>> Below this is for MESSAGE DISPOSAL <<*/
public:
    // Only for Pathway
    inline void SetNumDestination (uint32_t n_dest) { dispcount = n_dest; }

    // Only for Pathway and Module
    inline void Dispose () 
    {
        dispcount--;
        if (dispcount == 0)
            delete this;        // COMMIT SUICIDE!
    }

    // Universal
    inline Type GetType () { return type; }

private:
    Type type;
    uint32_t dispcount;
};
