#include <Component/TSManager.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Message/SignalMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

TSManager::TSManager (string iname, Component *parent, uint32_t num_boards)
    : Module ("TSManager", iname, parent, 1)
{
    IPORT_DynFin = CreatePort ("dynfin", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    IPORT_idle = CreatePort ("idle", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());
    IPORT_TSend = CreatePort ("remote_tsend", Module::PORT_INPUT,
            Prototype<SignalMessage>::Get());
    
    OPORT_TSParity = CreatePort ("ts_parity", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());
    OPORT_TSend = CreatePort ("tsend", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    dyn_fin = true;
    prop_idle = true;
    ts_parity = false;
    is_finish = true;
    is_start = true;
    is_end = false;

    this->num_boards = num_boards;
    this->end_counter = 0;

    this->cur_timestep = 0;

    ts_buf_ = 3;
}

void TSManager::Operation (Message **inmsgs, Message **outmsgs, Instruction *instr)
{
    IntegerMessage *fin_msg = static_cast<IntegerMessage*> (inmsgs[IPORT_DynFin]);
    IntegerMessage *idle_msg = static_cast<IntegerMessage*> (inmsgs[IPORT_idle]);
    SignalMessage *remote_end_msg = static_cast<SignalMessage*> (inmsgs[IPORT_TSend]);

    if(fin_msg)
    {
        bool val = fin_msg->value;
        if(!dyn_fin && val)
            INFO_PRINT("[TSM] Dynamics Finished, %p", fin_msg);
        
        dyn_fin = (val != 0)? 1:0;
        is_finish = dyn_fin && prop_idle;
    }
    if(idle_msg)
    {
        bool val = idle_msg->value;

        if(!prop_idle && val)
        {
            INFO_PRINT("[TSM] Propagator is idle, %p", idle_msg);
        }
        else if (prop_idle && !val)
        {
            INFO_PRINT("[TSM] Propagator is busy, %p", idle_msg);
        }
        
        prop_idle = (val != 0)? 1:0;
        is_finish = dyn_fin && prop_idle;
    }

    if(remote_end_msg)
    {
        if(likely(remote_end_msg->value))
            end_counter++;
        else
        {
            SIM_ERROR ("TS end value is zero", GetFullName().c_str());
            return;
        }
        
        INFO_PRINT ("[TSM] Get remote TS end signal");
    }

    if (is_finish && is_start && !is_end)
    {
        if(ts_buf_)
            ts_buf_--;
        else
        {
            INFO_PRINT ("[TSM] The board is finished");
            outmsgs[OPORT_TSend] = new SignalMessage (-1, true);
            end_counter++;

            ts_buf_ = 3;
            is_end = true;
        }
    }
    else if (!is_finish && !is_start && !dyn_fin)
        is_start = true;

    if ((end_counter == num_boards) && is_start)
    {
        ts_parity = !ts_parity;

        outmsgs[OPORT_TSParity] = new IntegerMessage (ts_parity);

        end_counter = 0;
        is_finish = false;
        cur_timestep++;
        is_start = false;
        is_end = false;

        INFO_PRINT ("[TSM] Current Timestep %d", cur_timestep);
        INFO_PRINT ("[TSM] Update Timestep parity %d to %d", !ts_parity, ts_parity);
    }
}
