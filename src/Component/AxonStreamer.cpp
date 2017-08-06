#include <Component/AxonStreamer.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/AxonMessage.h>
#include <Message/DramReqMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;

AxonStreamer::AxonStreamer (string iname, Component *parent, uint8_t io_buf_size)
    : Module ("AxonStreamer", iname, parent, 1)
{
    IPORT_Axon = CreatePort ("axon_in", Module::PORT_INPUT,
            Prototype<AxonMessage>::Get());
    OPORT_Addr = CreatePort ("addr_out", Module::PORT_OUTPUT,
            Prototype<DramReqMessage>::Get());
    OPORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    // DRAM Spec.
    read_bytes = io_buf_size; 

    // Parameter
    num_streamer_ = 4;

    for (int i=0; i<num_streamer_; i++)
    {
        streaming_task_.push_back(StreamJob());
        is_idle_.push_back(false);
        tag_counter_.push_back(i);

        free_list_.push_back(i);
    }

    stream_out_ = 0;
    ongoing_task_ = 0;
}

void AxonStreamer::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // Process input
    AxonMessage *axon_msg = static_cast<AxonMessage*>(inmsgs[IPORT_Axon]);

    if(axon_msg && !free_list_.empty())
    {
        int idx = free_list_.front();
        free_list_.pop_front();
        work_list_.push_back(idx);

        uint8_t tag = tag_counter_[idx]; 
        streaming_task_[idx] = 
            StreamJob (axon_msg->value, axon_msg->len,
                    axon_msg->value, tag_counter_[idx], axon_msg->is_inh);
        streaming_task_[idx].off_ofs = (axon_msg->target != -1)? true:false;

        is_idle_[idx] = false;

        INFO_PRINT("[AS] (%s) Start DRAM streaming (addr: %lx, len: %u, tag %d), stream_rr %d, task %d", 
                GetFullNameWOClass().c_str(),
                axon_msg->value, axon_msg->len, tag, idx, ongoing_task_);
        
        tag_counter_[idx] += num_streamer_;
    
        if(ongoing_task_ == 0)
            outmsgs[OPORT_idle] = new IntegerMessage (0);

        ongoing_task_++;

        if(unlikely(!axon_msg->len))
        {
            SIM_FATAL ("[AS] Axon length is zero", GetFullName().c_str());

        }
    }
    else if(axon_msg && free_list_.empty())
        inmsgs[IPORT_Axon] = nullptr;

   
    if(work_list_.empty())
    {
        delete outmsgs[OPORT_idle];
        outmsgs[OPORT_idle] = new IntegerMessage (1);
        return;
    }

    stream_out_ %= work_list_.size();
    uint8_t out_idx = work_list_[stream_out_];

    StreamJob job = streaming_task_[out_idx];
    bool streamer_idle = is_idle_[out_idx]; 
    
    if(!streamer_idle && (job.read_addr < job.base_addr + job.ax_len) &&
            (job.read_addr >= job.base_addr))
    {
        INFO_PRINT ("[AS] Send read request");
        if(job.read_addr == job.base_addr)
        {
            outmsgs[OPORT_Addr] = new DramReqMessage (0, job.read_addr, 
                    job.tag, job.ax_len, job.is_inh, job.off_ofs);
        }
        else
            outmsgs[OPORT_Addr] = new DramReqMessage (0, 
                    job.read_addr, job.tag, 0, job.is_inh);

        streaming_task_[out_idx].read_addr += read_bytes;
    }
    else if(!streamer_idle)
    {
        INFO_PRINT ("[AS] (%s) Finish DRAM streaming (addr: %u, len: %u)", 
                GetFullNameWOClass().c_str(),
                job.base_addr, job.ax_len);
        
        if(ongoing_task_ != 0)
            ongoing_task_--;

        else if(ongoing_task_ == 0)
        {
            delete outmsgs[OPORT_idle];
            outmsgs[OPORT_idle] = new IntegerMessage (1);
        }
        
        is_idle_[out_idx] = true;

        free_list_.push_back(out_idx);
        work_list_.erase (work_list_.begin() + stream_out_);
    }
    else
    {
        SIM_ERROR ("Unreached code", GetFullName().c_str());
    }


    stream_out_++;
}
