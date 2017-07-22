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

    reg_size_ = 64;
    for (int i=0; i<reg_size_; i++)
        internal_reg_.push_back (0);
    regFLA_ = 0;
    regPLA_ = 0;
    reg_head_ = 0;

    in_delay_ = 0;
    in_len_ = 0;
    in_axaddr_ = 0;

    delay_buf_ = reg_size_;
}

void DelayAxonMgr::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
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
            INFO_PRINT ("[DAM] INSERT state");
            state_ = INSERT;
        }
    }
    else if (!fetch_fin_)
    {
        INFO_PRINT ("[DAM] FETCH state");
        state_ = FETCH;

    }

    if(state_ == PROMOTE)
    {
        if(state_counter_ == 0)
        {
            INFO_PRINT ("[DAM] Delay axon mgr is PROMOTE state");
            // Read PLA and FLA, send read request to metadata storage
            readFLA_ = regFLA_;
            readPLA_ = regPLA_;

            if(readPLA_ == 0)
            {
                state_ = IDLE;
                state_counter_ = 0;
            }
            else
            {
                outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);
                state_counter_++;
            }

        }
        else if(state_counter_ == 1)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                uint16_t relative_delay = meta_msg->val16;
                if(relative_delay == 1)
                {
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
                    // Decrement relative delay
                    outmsgs[PORT_waddr] = new IndexMessage (0, readPLA_);
                    outmsgs[PORT_wdata] = 
                        new DelayMetaMessage (0, meta_msg->next_addr,
                                meta_msg->addr_sub, 
                                relative_delay-1);
                }

                state_ = IDLE;
                state_counter_ = 0;
                reg_head_++;
            }
        }
    }
    else if(state_ == RETRIEVE)
    {
        // TODO Case: Update PLA
        if(state_counter_ == 0)
        {
            // Read PLA and FLA, send read request to metadata storage
            readFLA_ = regFLA_;
            readPLA_ = regPLA_;

            outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);

            state_counter_++;
        }
        else if(state_counter_ == 1)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                delay_buf_ += meta_msg->val16;  

                if(delay_buf_ >= in_delay_)
                {
                    // Pass to insert state
                    state_ = INSERT;
                    state_counter_ = 1;

                    // Read free page
                    readFLA_ = regFLA_;
                    outmsgs[PORT_raddr] = new IndexMessage (0, readFLA_);

                    prehead_ptr_ = addr_ptr_;
                    addr_ptr_ = (delay_buf_ == in_delay_)? 
                                             meta_msg->addr_sub : 0;
                    delay_buf_ = reg_size_;
                }
                else
                {
                    addr_ptr_ = meta_msg->next_addr;
                    outmsgs[PORT_raddr] = new IndexMessage (0, addr_ptr_);
                }
            }
        }
    }
    else if(state_ == INSERT)
    {
        if(state_counter_ == 0) // Read free page
        {
            // Read FLA
            readFLA_ = regFLA_;
            outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);

            // Read DALA
            uint16_t reg_idx = (reg_head_ + in_delay_) % reg_size_;
            addr_ptr_ = internal_reg_[reg_idx]; 
                
            // Update DALA
            internal_reg_[reg_idx] = readFLA_;

            state_counter_ = 1;
        }
        else if(state_counter_ == 1)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                // Update FLA
                regFLA_ = meta_msg->next_addr;
                
                // Write data
                outmsgs[PORT_waddr] = new IndexMessage (0, readFLA_);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, addr_ptr_, in_axaddr_, in_len_);

                if(addr_ptr_ == 0)
                {
                    state_counter_ = 2;
                    addr_ptr_ = readFLA_;
                
                    // Read free entry (for header)
                    readFLA_ = regFLA_;
                    outmsgs[PORT_raddr] = new IndexMessage (0, readPLA_);
                }
                else
                {
                    state_ = IDLE;
                    state_counter_ = 0;
                }
            }
        }
        else if(state_counter_ == 2) // Over 255th case, write header
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                // Update FLA
                regFLA_ = meta_msg->next_addr;
                
                // Read preheader
                outmsgs[PORT_raddr] = new IndexMessage (0, prehead_ptr_);
                prePLE_.addr = prehead_ptr_; 

                state_counter_ = 3;
            }            
        }
        else if(state_counter_ == 3)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                // Store pre PLE
                prePLE_.next_ptr = meta_msg->next_addr;
                prePLE_.delay = meta_msg->val16;
                prePLE_.list_head = meta_msg->addr_sub;

                // Write new head pointer
                uint16_t delay = in_delay_ - prePLE_.delay;
                outmsgs[PORT_waddr] = new IndexMessage(0, readFLA_);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, prePLE_.next_ptr,
                                          addr_ptr_, delay);

                // Read post head pointer
                postPLE_.addr = prePLE_.next_ptr;
                if (postPLE_.addr != 0) 
                {
                    outmsgs[PORT_raddr] = 
                        new IndexMessage(0, postPLE_.addr);
                }

                state_counter_ = 4;
            }
        }
        else if(state_counter_ == 4)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
                if(unlikely(!postPLE_.addr))
                {
                    SIM_FATAL ("postPLE is not valid", GetFullName().c_str());
                    return;
                }

                // Store pre PLE
                postPLE_.next_ptr = meta_msg->next_addr;
                postPLE_.delay = meta_msg->val16;
                postPLE_.list_head = meta_msg->addr_sub;

                // Write new pre-head pointer
                outmsgs[PORT_waddr] = new IndexMessage (0, prePLE_.addr);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, addr_ptr_,
                                          prePLE_.list_head,
                                          prePLE_.delay);
                
                state_counter_ = 5;
            }
            else if(!postPLE_.addr)
            {
                // Write new pre-head pointer
                outmsgs[PORT_waddr] = new IndexMessage (0, prePLE_.addr);
                outmsgs[PORT_wdata] = 
                    new DelayMetaMessage (0, addr_ptr_,
                                          prePLE_.list_head,
                                          prePLE_.delay);

                state_ = IDLE;
                state_counter_ = 0;
            }

        }
        else if(state_counter_ == 5)
        {
            // Write new post-head pointer
            outmsgs[PORT_waddr] = new IndexMessage (0, postPLE_.addr);
            outmsgs[PORT_wdata] = 
                new DelayMetaMessage (0, addr_ptr_,
                                      postPLE_.list_head,
                                      postPLE_.delay);
        }
    }
    else if(state_ == FETCH)
    {
        if(state_counter_ == 0)
        {
            readFLA_ = regFLA_;
            readDALA_ = internal_reg_[reg_head_];

            if(readDALA_ == 0)
            {
                state_ = IDLE;
                state_counter_ = 0;
                fetch_fin_ = true;
            }
            else
            {
                outmsgs[PORT_raddr] = new IndexMessage (0, readDALA_); 
                state_counter_ = 1;
            }
        }
        else if(state_counter_ == 1)
        {
            DelayMetaMessage *meta_msg =
                static_cast<DelayMetaMessage*> (inmsgs[PORT_rdata]);

            if(meta_msg)
            {
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
                state_counter_ = 0;
            }           

        }
    }
}


