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
            Prototype<SignalMessage>::Get());
    OPORT_TSend = CreatePort ("tsend", Module::PORT_OUTPUT,
            Prototype<SignalMessage>::Get());

    dyn_fin = true;
    prop_idle = true;
    ts_parity = false;
    is_finish = true;
    is_start = true;

    this->num_boards = num_boards;
    this->end_counter = 0;

    this->cur_timestep = 0;
}

void TSManager::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    IntegerMessage *fin_msg = static_cast<IntegerMessage*> (inmsgs[IPORT_DynFin]);
    IntegerMessage *idle_msg = static_cast<IntegerMessage*> (inmsgs[IPORT_idle]);
    SignalMessage *remote_end_msg = static_cast<SignalMessage*> (inmsgs[IPORT_TSend]);

    if(fin_msg)
    {
        bool val = fin_msg->value;
        dyn_fin = (val != 0)? 1:0;

        if(dyn_fin)
            DEBUG_PRINT("[TSM] Dynamics Finished, %p", fin_msg);

        is_finish = dyn_fin && prop_idle;
    }
    if(idle_msg)
    {
        bool val = idle_msg->value;
        prop_idle = (val != 0)? 1:0;

        if(prop_idle)
        {
            DEBUG_PRINT("[TSM] Propagator is idle, %p", idle_msg);
        }
        else
        {
            DEBUG_PRINT("[TSM] Propagator is busy, %p", idle_msg);
        }
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
        
        DEBUG_PRINT ("[TSM] Get remote TS end signal");
    }

    if (is_finish && is_start)
    {
        DEBUG_PRINT ("[TSM] The chip is finished");
        outmsgs[OPORT_TSend] = new SignalMessage (-1, true);
        end_counter++;
    }
    else if (!is_finish && !is_start && !dyn_fin)
        is_start = true;

    if ((end_counter == num_boards) && is_start)
    {
        ts_parity = !ts_parity;

        outmsgs[OPORT_TSParity] = new SignalMessage (-1, ts_parity);

        end_counter = 0;
        is_finish = false;
        cur_timestep++;
        is_start = false;

        DEBUG_PRINT ("[TSM] Current Timestep %d", cur_timestep);
        DEBUG_PRINT ("[TSM] Update Timestep parity %d to %d", !ts_parity, ts_parity);

    }
}
