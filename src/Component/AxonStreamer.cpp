#include <Component/AxonStreamer.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/AxonMessage.h>
#include <Message/IndexMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

AxonStreamer::AxonStreamer (string iname, Component *parent, uint8_t io_buf_size)
    : Module ("AxonStreamer", iname, parent, 1)
{
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Addr = CreatePort ("addr_out", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    // DRAM Spec.
    read_bytes = io_buf_size; 

    // Parameter
    num_streamer_ = 1;

    for (int i=0; i<num_streamer_; i++)
    {
        streaming_task_.push_back(StreamJob());
        is_idle_.push_back(false);
        tag_counter_.push_back(i);
    }

    stream_rr_ = 0;
    ongoing_task_ = 0;
}

void AxonStreamer::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    bool streamer_idle = is_idle_[stream_rr_]; 

    if(streamer_idle && axon_msg)
    {
        uint8_t tag = tag_counter_[stream_rr_]; 
        streaming_task_[stream_rr_] = 
            StreamJob (axon_msg->value, axon_msg->len,
                    axon_msg->value, tag_counter_[stream_rr_]);

        is_idle_[stream_rr_] = false;

        INFO_PRINT("[AS] Start DRAM streaming (addr: %lu, len: %u), stream_rr %d, task %d", 
                axon_msg->value, axon_msg->len, stream_rr_, ongoing_task_);
        
        tag_counter_[stream_rr_] += num_streamer_;
    
        if(ongoing_task_ == 0)
            outmsgs[OPORT_idle] = new IntegerMessage (0);

        ongoing_task_++;
    }
    else if(!streamer_idle && axon_msg)
        inmsgs[IPORT_Axon] = nullptr;

    StreamJob job = streaming_task_[stream_rr_];
    streamer_idle = is_idle_[stream_rr_]; 
    
    if(!streamer_idle && (job.read_addr < job.base_addr + job.ax_len))
    {
        INFO_PRINT ("[AS] Send read request");
        outmsgs[OPORT_Addr] = new IndexMessage (0, job.read_addr, job.tag);
       
        streaming_task_[stream_rr_].read_addr += read_bytes;
    }
    else if(!streamer_idle)
    {
        INFO_PRINT ("[AS] Finish DRAM streaming (addr: %u, len: %u)", job.base_addr, job.ax_len);
        
        if(ongoing_task_ != 0)
            ongoing_task_--;

        if(ongoing_task_ == 0)
            outmsgs[OPORT_idle] = new IntegerMessage (1);
        
        is_idle_[stream_rr_] = true;
    }


    stream_rr_ = (stream_rr_ + 1) % num_streamer_;
}
