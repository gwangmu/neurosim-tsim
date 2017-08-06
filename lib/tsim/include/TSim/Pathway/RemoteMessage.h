#pragma once

#include <TSim/Pathway/Message.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

// NOTE: derive this class to make a RemoteMessage
namespace TSim
{
    struct RemoteMessage: public Message
    {
    public:
        RemoteMessage (const char *clsname) 
            : Message (clsname), DST_ID0 (-1), DST_ID1 (-1) {}
    
        RemoteMessage (const char *clsname, uint32_t payload_size, 
                uint32_t dst_id0, uint32_t dst_id1 = 0)
            : Message (clsname, -1, payload_size), 
              DST_ID0 (dst_id0), DST_ID1 (dst_id1)
        {
            if (payload_size == 0)
                DESIGN_FATAL ("RemoteMessage cannot be zero-sized",
                        "RemoteMessage");
        }
    
        virtual RemoteMessage* Clone () = 0;
    
    public:
        const uint32_t DST_ID0;
        const uint32_t DST_ID1;
    };
}
