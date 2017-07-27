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

using namespace std;

PacketDecoder::PacketDecoder (string iname, Component *parent)
    : Module ("PacketDecoder", iname, parent, 1)
{
    IPORT_Packet = CreatePort ("packet", Module::PORT_INPUT,
            Prototype<PacketMessage>::Get());
    
    OPORT_TSEnd = CreatePort ("ts_end", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());
    OPORT_Axon = CreatePort ("axon", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
}

void PacketDecoder::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    PacketMessage *pkt_msg = static_cast<PacketMessage*> (inmsgs[IPORT_Packet]);

    if(pkt_msg)
    {
       PacketType type = pkt_msg->type;
       if(type == TSEND)
       {
           INFO_PRINT ("[PkD] Receive remote TS end message");
           outmsgs[OPORT_TSEnd] = new SignalMessage (0, pkt_msg->value);
       }
       else if(type == AXON)
       {
           INFO_PRINT ("[PkD] Receive Axon message");
           outmsgs[OPORT_Axon] = new AxonMessage (0, pkt_msg->value,
                   pkt_msg->val16);
       }
    }
}
