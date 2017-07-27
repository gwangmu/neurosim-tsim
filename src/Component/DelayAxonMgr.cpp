#include <Component/DelayAxonMgr.h>

#include <Message/DelayMessage.h>
#include <Message/DelayMetaMessage.h>
#include <Message/SignalMessage.h>
#include <Message/IndexMessage.h>
#include <Message/AxonMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

DelayAxonMgr::DelayAxonMgr (string iname, Component* parent)
    : Module ("DelayAxonMgr", iname, parent, 1)
{
    PORT_input = CreatePort ("delay_input", Module::PORT_INPUT,
            Prototype<DelayMessage>::Get());
    PORT_TSparity = CreatePort ("curTS", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    PORT_output = CreatePort ("axon_output", Module::PORT_OUTPUT,
            Prototype<AxonMessage>::Get());
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());

    PORT_raddr = CreatePort ("raddr", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    PORT_rdata = CreatePort ("rdata", Module::PORT_INPUT,
            Prototype<DelayMetaMessage>::Get());

    PORT_waddr = CreatePort ("waddr", Module::PORT_OUTPUT,
            Prototype<IndexMessage>::Get());
    PORT_wdata = CreatePort ("wdata", Module::PORT_OUTPUT,
            Prototype<DelayMetaMessage>::Get());

    state_ = IDLE;
    state_counter_ = 0;
    ts_parity_ = false;
    fetch_fin_ = true;
    is_idle_ = false;

    reg_size_ = 64;
    for (int i=0; i<reg_size_; i++)
        internal_reg_.push_back (0);
    regFLA_ = 1;
    regPLA_ = 0;
    reg_head_ = 0;

    in_delay_ = 0;
    in_len_ = 0;
    in_axaddr_ = 0;

    delay_buf_ = reg_size_;

    entry_cnt_ = 0;
}

void DelayAxonMgr::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    // Change STATE

    IntegerMessage *ts_msg = 
        static_cast<IntegerMessage*> (inmsgs[PORT_TSparity]);
    DelayMessage *input_msg =
        static_cast<DelayMessage*> (inmsgs[PORT_input]);

    if(ts_msg && (ts_parity_ != ts_msg->value))
    {
        if(state_ != IDLE)
        {
            SIM_ERROR("Delay manager is not idle, but TSparity is changed",
                    GetFullName().c_str());
            return;
        }

        ts_parity_ = ts_msg->value;

        inmsgs[PORT_input] = nullptr;

        fetch_fin_ = false;

        INFO_PRINT ("[DAM] PROMOTE state");
        state_ = PROMOTE;
    }  
    else if(state_ != IDLE)
    {
        inmsgs[PORT_input] = nullptr;
    }
    else if(input_msg)
    {
        in_delay_ = input_msg->delay; 
        in_len_ = input_msg->len;
        in_axaddr_ = input_msg->value;

        if(in_delay_ > (reg_size_-1))
        {
            INFO_PRINT ("[DAM] RETRIEVE state");
            state_ = RETRIEVE;
        }
        else
        {
            INFO_PRINT ("[DAM] INSERT state (delay %d, addr %lx(%lx), len %d)",
                         in_delay_, in_axaddr_, input_msg->value, in_len_);
            state_ = INSERT;
        }
    }
    else if (!fetch_fin_ && (GetOutQueSize(PORT_output) < 2))
    {
        INFO_PRINT ("[DAM] FETCH state");
        state_ = FETCH;
    }
    
    if(state_ == IDLE && fetch_fin_)
    {
        if(!is_idle_)
        {
            INFO_PRINT ("[DAM] Delay Axon manager is idle %d",
                    is_idle_);
            outmsgs[PORT_idle] = new IntegerMessage (1);
            is_idle_ = true;
        }
    }
    else if(state_ != IDLE || !fetch_fin_)
    {
        if(is_idle_)
        {
            INFO_PRINT ("[DAM] Delay Axon manager is busy");
            outmsgs[PORT_idle] = new IntegerMessage (0);
            is_idle_ = false;
        }
    }

    if(state_ == PROMOTE)
    {
        state_counter_ = Promote (inmsgs, outmsgs, state_counter_);
    }
    else if(state_ == RETRIEVE)
    {
        state_counter_ = Retrieve (inmsgs, outmsgs, state_counter_);
    }
    else if(state_ == INSERT)
    {
        state_counter_ = Insert (inmsgs, outmsgs, state_counter_);
    }
    else if(state_ == FETCH)
    {
        state_counter_ = Fetch (inmsgs, outmsgs, state_counter_);
    }
}

int16_t DelayAxonMgr::Promote (Message **inmsgs, Message **outmsgs,
                               int state)
{
    // read PLA
    if (state == 0)
    {
        if (regPLA_ == 0)
        {
            INFO_PRINT ("[DAM] PLA is nullptr");
            state_ = IDLE;

            reg_head_ = (reg_head_ + 1) % reg_size_;
            return 0;
        }

        INFO_PRINT ("[DAM] Delay axon mgr is PROMOTE state");
        // Read PLA and FLA, send read request to metadata storage
        readPLA_ = regPLA_;
        readFLA_ = regFLA_;
        outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);
        return 1;
    }
    // 
    else if (state == 1)
    {
        DelayMetaMessage *meta_msg =
            static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);
        
        if(meta_msg)
        {
            uint16_t relative_delay = meta_msg->val16;
            if(relative_delay == 1)
            {
                INFO_PRINT ("[DAM] Update 255th DALA");

                // Update last DALA (Delayed Axon List Address)
                uint32_t last_dala = meta_msg->addr_sub;
                internal_reg_[reg_head_] = last_dala;

                // Update new free page
                outmsgs[PORT_waddr] = 
                    new IndexMessage (0, readPLA_);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage(0, readFLA_, 0, 0);

                // Update PLA and FLA
                regFLA_ = readPLA_;
                regPLA_ = meta_msg->next_addr;
            }
            else
            {
                INFO_PRINT ("[DAM] Decrement relative delay (%u)",
                             relative_delay);

                // Decrement relative delay
                outmsgs[PORT_waddr] = new IndexMessage (0, readPLA_);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, meta_msg->next_addr,
                            meta_msg->addr_sub, 
                            relative_delay-1);
            }
        }
            
        state_ = IDLE;

        reg_head_ = (reg_head_ + 1) % reg_size_;
        return 0;
    }


    return -1;
}

int16_t DelayAxonMgr::Retrieve (Message **inmsgs, Message **outmsgs,
                               int state)
{
    // Read PLA
    if(state == 0)
    {
        INFO_PRINT ("[DAM] Retrieve state read PLA");

        // Read PLA and FLA, send read request to metadata storage
        readFLA_ = regFLA_;
        readPLA_ = regPLA_;

        // Reset pre-PLE
        prePLE_ = PLE();
        postPLE_ = PLE();

        if(readPLA_)
        {
            outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);
            delay_buf_ = reg_size_;
            return 1;
        }
        else
        {
            outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);
            regPLA_ = readFLA_;
            return 2;
        }
    }
    // Find corresponding pointer list
    else if (state == 1)
    {
        DelayMetaMessage *meta_msg =
            static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

        if(meta_msg)
        {
            delay_buf_ += meta_msg->val16;  

            if(delay_buf_ == in_delay_) // Found it!
            {
                // Update head pointer
                outmsgs[PORT_waddr] = new IndexMessage (0, addr_ptr_);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, meta_msg->next_addr,
                                          readFLA_,
                                          meta_msg->val16);
                
                // Read free page
                outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);
                addr_ptr_ = meta_msg->addr_sub;

                // Pass to insert state
                state_ = INSERT;
                return 1;
            }
            else if(delay_buf_ > in_delay_)
            {
                // Update post PLE
                postPLE_ = PLE (addr_ptr_, meta_msg->next_addr, 
                                delay_buf_, meta_msg->addr_sub);

                // Read free page
                outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);
           
                // Write post-PLE
                uint16_t rel_delay = delay_buf_ - in_delay_;
                outmsgs[PORT_waddr] = new IndexMessage (0, addr_ptr_);
                outmsgs[PORT_wdata] =
                    new DelayMetaMessage (0, postPLE_.next_ptr,
                                          rel_delay,
                                          postPLE_.list_head);

                return 2;
            }
            else
            {
                prePLE_ = PLE (addr_ptr_, meta_msg->next_addr, 
                        delay_buf_, meta_msg->addr_sub);
                prePLE_.delay = meta_msg->val16;

                addr_ptr_ = meta_msg->next_addr;
                
                // Read next list entry
                if(addr_ptr_)
                {
                    outmsgs[PORT_raddr] = new IndexMessage (0, addr_ptr_);
                    return 1;
                }
                else // End of Pointer list
                {
                    // Get free entry
                    postPLE_ = PLE();
                    
                    outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);
                    return 2;
                }
            }
        }
    }
    // Write new head pointer 
    else if (state == 2)
    {
        DelayMetaMessage *meta_msg =
            static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

        // List head is nullptr
        addr_ptr_ = 0;

        if(meta_msg)
        {
            // Update FLA
            regFLA_ = meta_msg->next_addr;
            if(!regFLA_)
                regFLA_ = readFLA_ + 1;

            // Write new PLE
            // Update list head before write it
            outmsgs[PORT_waddr] = new IndexMessage (0, readFLA_);
            outmsgs[PORT_wdata] = 
                new DelayMetaMessage (0, postPLE_.addr,
                                      in_delay_ - prePLE_.abs_delay, regFLA_);
      

            if(prePLE_.addr)
                return 3;
            else
            {
                state_ = IDLE;
                return 0;
            }
        }

        return 2;
    }
    else if(state == 3)
    {
        // Write pre PLE
        outmsgs[PORT_waddr] = new IndexMessage(0, prePLE_.addr);
        outmsgs[PORT_wdata] =
            new DelayMetaMessage (0, readFLA_, 
                                  prePLE_.delay, prePLE_.list_head);

    
        state_ = IDLE;
        return 0;
    }

    return -1;
}

int16_t DelayAxonMgr::Insert (Message **inmsgs, Message **outmsgs,
                               int state)
{
    // Read free entry
    if(state == 0)
    {
        INFO_PRINT("[DAM] INSERT-0 state (FLA: %u)", regFLA_);
        
        // Read FLA
        readFLA_ = regFLA_;
        outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);

        // Read DALA
        uint16_t reg_idx = (reg_head_ + in_delay_) % reg_size_;
        addr_ptr_ = internal_reg_[reg_idx]; 
            
        // Update DALA
        internal_reg_[reg_idx] = readFLA_;

        return 1;
    }
    else if(state == 1)
    {
        DelayMetaMessage *meta_msg =
            static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

        if(meta_msg)
        {
            INFO_PRINT ("[DAM] Insert-1 %x %lx %u", 
                    addr_ptr_, in_axaddr_, in_len_);
            
            // Update FLA
            regFLA_ = meta_msg->next_addr;
            if(!regFLA_)
                regFLA_ = readFLA_ + 1;

            // Write data
            outmsgs[PORT_waddr] = new IndexMessage (0, readFLA_);
            outmsgs[PORT_wdata] = 
                new DelayMetaMessage (0, addr_ptr_, in_axaddr_, in_len_);
        
            state_ = IDLE;
            return 0;
        }      

        return 1;
    }


    return -1;
}

int16_t DelayAxonMgr::Fetch (Message **inmsgs, Message **outmsgs,
                               int state)
{
    if (state == 0)
    {
        readFLA_ = regFLA_;
        readDALA_ = internal_reg_[reg_head_];
        
        INFO_PRINT ("[DAM] Fetch-0 (FLA %u DALA %u)", readFLA_, readDALA_);

        if(readDALA_ == 0)
        {
            state_ = IDLE;
            fetch_fin_ = true;
            return 0;
        }
        else
        {
            outmsgs[PORT_raddr] = new IndexMessage (0, readDALA_);
            return 1;
        }
    }
    else if (state == 1)
    {
        DelayMetaMessage *meta_msg =
            static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

        if(meta_msg)
        {
            INFO_PRINT ("[DAM] Fetch-1 (DALA %x, addr %lx, len %d)", 
                    meta_msg->next_addr, meta_msg->addr_sub,
                    meta_msg->val16);
            
            // New FL elem.
            outmsgs[PORT_waddr] = new IndexMessage (0, readDALA_);
            outmsgs[PORT_wdata] = 
                new DelayMetaMessage (0, readFLA_, 0, 0);

            // Output
            outmsgs[PORT_output] = 
                new AxonMessage (0, meta_msg->addr_sub, 
                                meta_msg->val16);
          
            // Update 0th DALA / FLA
            internal_reg_[reg_head_] = meta_msg->next_addr;
            regFLA_ = readDALA_; 
           
            state_ = IDLE;
            return 0; 
        }

        return 1;
    }

    return -1;
}



