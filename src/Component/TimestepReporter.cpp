#include <Component/TimestepReporter.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/PacketMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

TimestepReporter::TimestepReporter (string iname, Component *parent, 
        uint32_t num_boards)
    : Module ("TimestepReporter", iname, parent, 1)
{
    IPORT_TSend = CreatePort ("remote_tsend", Module::PORT_INPUT,
            Prototype<PacketMessage>::Get());
    
    this->num_boards = num_boards;
    this->end_counter = 0;
    this->cur_timestep = 0;
}

void TimestepReporter::Operation (Message **inmsgs, Message **outmsgs, 
        Instruction *instr)
{
    PacketMessage *end_msg = static_cast<PacketMessage*> (inmsgs[IPORT_TSend]);

    if (end_msg && end_msg->type == PacketMessage::TSEND) 
        end_counter++;

    if (end_counter == num_boards)
    {
        end_counter = 0;
        cur_timestep++;

        PRINT ("Timestep %d..", cur_timestep);
    }
}
