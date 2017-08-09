#include <Component/PacketDecoder.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Message/SelectMessage.h>
#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <random>

using namespace std;
using namespace TSim;

USING_TESTBENCH;

PacketDecoder::PacketDecoder (string iname, Component *parent)
    : Module ("PacketDecoder", iname, parent, 1)
{
    num_propagators_ = GET_PARAMETER(num_propagators);
    
    IPORT_Packet = CreatePort ("packet", Module::PORT_INPUT,
            Prototype<PacketMessage>::Get());
    
    OPORT_TSEnd = CreatePort ("ts_end", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    for(int i=0; i<num_propagators_; i++)
    {
        OPORT_Axons.push_back(CreatePort ("axon" + to_string(i), 
                              Module::PORT_OUTPUT,
                              Prototype<AxonMessage>::Get()));
        OPORT_Bypass.push_back(CreatePort ("bypass" + to_string(i), 
                              Module::PORT_OUTPUT,
                              Prototype<AxonMessage>::Get()));
    }
}

void PacketDecoder::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
{
    PacketMessage *pkt_msg = static_cast<PacketMessage*> (inmsgs[IPORT_Packet]);

    if(pkt_msg)
    {
        PacketMessage::Type type = pkt_msg->type;

        if (type == PacketMessage::TSEND)
        {
            INFO_PRINT ("[PkD] %s Receive remote TS end message",
                    GetFullNameWOClass().c_str());
            outmsgs[OPORT_TSEnd] = new SignalMessage (0, 1);
        }
        else if (type == PacketMessage::AXON)
        {
            int target_idx = rand() % num_propagators_;

            INFO_PRINT ("[PkD] %s Receive Axon message",
                    GetFullNameWOClass().c_str());

            if(GetOutQueSize(OPORT_Axons[target_idx]) > 
                    0.5 * GetOutQueCapacity(OPORT_Axons[target_idx]))
            {
                outmsgs[OPORT_Bypass[target_idx]] = 
                    new AxonMessage (0, pkt_msg->addr,
                    pkt_msg->len/*, pkt_msg->delay*/);
            }
            else
            {
                outmsgs[OPORT_Axons[target_idx]] = 
                    new AxonMessage (0, pkt_msg->addr,
                    pkt_msg->len/*, pkt_msg->delay*/);
            }
        }
        else
            SYSTEM_ERROR ("bogus PacketMessage type");
    }

    return;
}
