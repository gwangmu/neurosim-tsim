#include <TSim/Module/RouterNetwork.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/RemoteMessage.h>
#include <TSim/Pathway/Link.h>
#include <BookSim2/booksim.hpp>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cinttypes>
#include <string>
#include <vector>
#include <cmath>
#include <functional>

using namespace std;
using namespace TSim;

RouterNetwork::RouterNetwork (string iname, Component *parent, string clock,
        RemoteMessage *msgproto, string topology, uint32_t quesize, uint32_t netsize)
    : Module ("RouterNetwork", iname, parent, 1)
{
    using namespace placeholders;

    SetClock (clock);

    if (netsize == 0)
        DESIGN_FATAL ("zero endpoint not allowed", GetFullName().c_str());
    else if (netsize > MAX_NETSIZE)
        DESIGN_FATAL ("over %u endpoints not allowed", GetFullName().c_str(), MAX_NETSIZE);
    this->netsize = netsize;

    if (quesize == 0)
        DESIGN_FATAL ("zero quesize not allowed", GetFullName().c_str());
    this->quesize = quesize;

    for (auto i = 0; i < netsize; i++)
    {
        PORT_RX[i] = CreatePort ("rx" + to_string(i), Module::PORT_INPUT, msgproto);
        PORT_TX[i] = CreatePort ("tx" + to_string(i), Module::PORT_OUTPUT, msgproto);
    }

    //for (uint32_t x = 0; x < netsize; x++)
    //    arrived_msgs[x] = nullptr;

    traffic_size_bits = 0;

    if (topology == "torus")
        nedges = netsize * 2;
    else
        nedges = -1;

    // NOTE: create temporary config file
    ofstream configfile;
    configfile.open (".booksim.conf");
    configfile << "topology = " << topology << ";" << endl;
    configfile << "k = " << (uint32_t)ceil(sqrt(netsize)) << ";" << endl;
    configfile << "n = 2" << ";" << endl << endl;
    configfile << "routing_function = dim_order" << ";" << endl;
    configfile << "sim_type = throughput" << ";" << endl;
    configfile << "num_vcs = 16" << ";" << endl;
    configfile << "vc_buf_size = 64" << ";" << endl;
    configfile << "sim_power = 1" << ";" << endl;
    configfile << "input_speedup = 10" << ";" << endl;
    configfile << "output_speedup = 10" << ";" << endl;
    configfile << "subnets = 2" << ";" << endl;
    configfile << "read_request_subnet = 0" << ";" << endl;
    configfile << "read_reply_subnet = 1" << ";" << endl;
    configfile << "write_request_subnet = 0" << ";" << endl;
    configfile << "write_reply_subnet = 1" << ";" << endl;
    configfile << "wait_for_tail_credit = 0" << ";" << endl;
    configfile << "read_request_begin_vc = 0" << ";" << endl;
    configfile << "read_request_end_vc = 0" << ";" << endl;
    configfile << "read_reply_begin_vc = 0" << ";" << endl;
    configfile << "read_reply_end_vc = 0" << ";" << endl;
    configfile << "write_request_begin_vc = 0" << ";" << endl;
    configfile << "write_request_end_vc = 0" << ";" << endl;
    configfile << "write_reply_begin_vc = 0" << ";" << endl;
    configfile << "write_reply_end_vc = 0" << ";" << endl;
    configfile << "output_buffer_size = 128" << ";" << endl;

    configfile << "traffic = uniform" << ";" << endl;
    configfile << "injection_rate = 0.15" << ";" << endl;
    configfile.close();

    BookSim_Initialize (".booksim.conf");
    BookSim_RegisterPacketArrivalCallback (bind (&RouterNetwork::PacketArrivalCallback, 
                this, _1, _2, _3, _4));

    // NOTE: clear temporary file
    remove (".booksim.conf");
}

bool RouterNetwork::IsValidConnection (Port *port, Endpoint *endpt)
{
    Module::IsValidConnection (port, endpt);

    if (endpt->GetEndpointType() != Endpoint::CAP)
    {
        if (!dynamic_cast<Link *>(endpt->GetParent()))
            DESIGN_FATAL ("must be connected with Link", GetName().c_str());

        if (port->iotype == Unit::PORT_INPUT)
        {
            DESIGN_INFO ("resetting input queue size to %u..",
                    GetName().c_str(), quesize);
            endpt->SetCapacity (quesize);
        }

        if (port->iotype == Unit::PORT_OUTPUT)
        {
            DESIGN_INFO ("resetting output queue size to %u..",
                    GetName().c_str(), quesize);
            endpt->SetCapacity (quesize);
        }
    }

    return true;
}


void RouterNetwork::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    bool has_pending_msgs = false;
    bool has_full_outque = false;
    bool outque_ready[MAX_NETSIZE];

    uint16_t remain_outque[MAX_NETSIZE];
    uint16_t min_remain_outque = -1;

    for (uint32_t x = 0; x < netsize; x++)
    {
        uint32_t flight_msg = msgqueue[x].size() + 
                              arrived_msg_queue[x].size() + 
                              GetOutQueSize(PORT_TX[x]) + 1;

        if (!msgqueue[x].empty())
            has_pending_msgs = true;
        
        if (unlikely (flight_msg > GetOutQueCapacity(PORT_TX[x])))
            SYSTEM_ERROR ("router output queue exploded %d /%d ",
                           msgqueue[x].size(),
                           msgqueue[x].size() + GetOutQueSize(PORT_TX[x]));

        remain_outque[x] = GetOutQueCapacity(PORT_TX[x]) - flight_msg;
        if(remain_outque[x] < min_remain_outque)
            min_remain_outque = remain_outque[x];
        
        //if (msgqueue[x].size() + GetOutQueSize(PORT_TX[x]) >= 
        //        GetOutQueCapacity(PORT_TX[x]))
        //{
        //    has_full_outque = true;
        //    outque_ready[x] = false;
        //}
        //else
        //    outque_ready[x] = true;
    }

    for (uint32_t x = 0; x < netsize; x++)
    {
        RemoteMessage *inmsg_casted;
        inmsg_casted = static_cast<RemoteMessage *>(inmsgs[PORT_RX[x]]);

        if (inmsg_casted)
        {
            uint32_t xdst = inmsg_casted->DST_ID0;
            bool is_broadcast = (xdst == -1);

            if ((is_broadcast && (min_remain_outque > 0)) ||
                    (!is_broadcast && (remain_outque[xdst] > 0)))
            {
                if (is_broadcast)
                {
                    for (uint32_t xx = 0; xx < netsize; xx++)
                        if (xx != x)
                        {
                            RemoteMessage *msgclone = inmsg_casted->Clone();
                            msgclone->Recycle();
                            msgqueue[xx].push (msgclone);
                            remain_outque[xx] -= 1;
                            
                            BookSim_GeneratePacket (x, xx, msgclone->BIT_WIDTH / FLIT_SIZE);
                        }

                    min_remain_outque -= 1;
                }
                else
                {
                    inmsg_casted->Recycle();
                    msgqueue[xdst].push (inmsg_casted);
                
                    remain_outque[xdst] -= 1;
                    if(remain_outque[xdst] < min_remain_outque)
                        min_remain_outque = remain_outque[xdst];

                    BookSim_GeneratePacket (x, xdst, inmsg_casted->BIT_WIDTH / FLIT_SIZE);
                }
            }
            else
            {
                inmsg_casted = nullptr;
                inmsgs[PORT_RX[x]] = nullptr;
            }
        }
    }

    // NOTE: clock Mhz * 8 (250MHz->2000Mhz)
    for (uint32_t i = 0; i < 8; i++)
    {
        if (has_pending_msgs)
            BookSim_Advance();
    }

    // NOTE: PacketArrivalCallback is expected to be called at this point.

    if (msg_arrived)
    {
        for (uint32_t x = 0; x < netsize; x++)
        {
            if(!arrived_msg_queue[x].empty())
            {
                outmsgs[PORT_TX[x]] = arrived_msg_queue[x].front();
                arrived_msg_queue[x].pop();
            }
            //arrived_msgs[x] = nullptr;
        }

        msg_arrived = false;
    }
}

void RouterNetwork::PacketArrivalCallback (uint32_t id, uint32_t latency,
        uint32_t srcid, uint32_t dstid)
{
    RemoteMessage *msg = msgqueue[dstid].front();
    msgqueue[dstid].pop();
    arrived_msg_queue[dstid].push(msg); //= msg;

    traffic_size_bits += msg->BIT_WIDTH;
    msg_arrived = true;
}
