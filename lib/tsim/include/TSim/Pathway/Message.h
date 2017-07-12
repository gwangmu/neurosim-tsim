#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Utility/StaticBranchPred.h>
#include <cinttypes>

using namespace std;

struct Message: public Metadata
{
public:
    // NOTE: keep DEST_RHS_ID==-1 if TOGGLE type
    enum Type { PLAIN, TOGGLE };

    // For prototype creation. 
    // DO NOT USE FOR ORDINARY MESSAGE CREATION
    Message (const char *clsname)
        : Metadata (clsname, ""), DEST_RHS_ID (-1), type (PLAIN), 
          dispcount (-1), BIT_WIDTH (0) {}

    Message (const char *clsname, uint32_t destrhsid, uint32_t bitwidth = 0)
        : Metadata (clsname, ""), DEST_RHS_ID (destrhsid), type (PLAIN), 
          dispcount (-1), BIT_WIDTH (bitwidth) {}

    Message (const char *clsname, Type type, uint32_t destrhsid, uint32_t bitwidth = 0)
        : Metadata (clsname, ""), DEST_RHS_ID (destrhsid), type (type), 
          dispcount (-1), BIT_WIDTH (bitwidth) {}

    uint32_t BIT_WIDTH;         // NOTE: 0 by default, CONST
    uint32_t DEST_RHS_ID;       // NOTE: -1 to broadcast

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
    // Only for Module
    inline void Recycle ()
    {
        dispcount = -1;         // INVINCIBLE
    }

    // Only for Pathway
    inline void SetNumDestination (uint32_t n_dest) 
    { 
        if (dispcount == -1)
            dispcount = n_dest; 
        else
            dispcount += n_dest - 1;
    }

    // Only for Pathway and Module
    inline void Dispose () 
    {
        if (likely (dispcount != -1))
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
