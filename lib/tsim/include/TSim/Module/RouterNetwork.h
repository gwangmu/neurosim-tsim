#pragma once

#include <TSim/Module/Module.h>
#include <TSim/Simulation/Testbench.h>
#include <TSim/Pathway/RemoteMessage.h>

#include <queue>
#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

namespace TSim
{
    class RemoteMessage;

    class RouterNetwork: public Module
    {
    public:
        static const uint32_t MAX_NETSIZE = 1024;
        static const uint32_t FLIT_SIZE = 16;       // NOTE: in bits
    
    public:
        RouterNetwork (string iname, Component *parent, string topology, 
                RemoteMessage *msgproto, string clock, uint32_t quesize, 
                uint32_t netsize);
    
        virtual bool IsValidConnection (Port *port, Endpoint *endpt);
        virtual void Operation (Message **inmsgs, Message **outmsgs, 
                Instruction *instr);
    
        uint64_t GetAccumTrafficBytes () { return traffic_size_bits / 8; }
        uint32_t GetNumEdges () { return nedges; }
    
    private:
        RemoteMessage *msgproto;
        uint32_t quesize;
        uint32_t netsize;
    
        uint32_t PORT_TX[MAX_NETSIZE];
        uint32_t PORT_RX[MAX_NETSIZE];
        queue<RemoteMessage *> msgqueue[MAX_NETSIZE];
    
        // state
        uint32_t nedges;
        std::queue<RemoteMessage *> arrived_msg_queue[MAX_NETSIZE];
        bool msg_arrived;
        uint64_t traffic_size_bits;
    
        void PacketArrivalCallback (uint32_t id, uint32_t latency,
                uint32_t srcid, uint32_t dstid);
    };
}
