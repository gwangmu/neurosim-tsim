#include <TSim/Device/Mux.h>
#include <TSim/Base/Component.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Mux::Mux (string iname, Component *parent, Message *msgproto, uint32_t ninput)
    : Device ("Mux", iname, parent, msgproto)
{
    // NOTE: IDs should be 0, 1, 2, ...
    for (int i = 0; i < ninput; i++)
        CreatePort (string("input") + to_string(i), Device::PORT_INPUT);

    // NOTE: ID should be 0
    CreatePort (string("select"), Device::PORT_CONTROL);
}


void Mux::PreClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("open/close pre-mux pathways");

    operation ("update control messages")
    {
        if (IntegerMessage *intmsg = 
                static_cast<IntegerMessage *>(ctrlports[0].endpt->Peek ()))
            nextctrlmsgs[0] = *intmsg;
    }

    operation ("open/close pre-mux pathways")
    {
        for (auto i = 0; i < ninports; i++)
        {
            uint64_t select = nextctrlmsgs[0].value;

            if (i != select)
                inports[i].endpt->SetExtReadyState (false);
            else
                inports[i].endpt->SetExtReadyState 
                    (!outport.endpt->IsBroadcastBlocked ());
        }
    }
}

void Mux::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("calc '%s'", GetFullName().c_str());

    operation ("peak messages from RHS")
    {
        for (auto i = 0; i < ninports; i++)
        {
            if (!nextinmsgs[i])
            {
                nextinmsgs[i] = inports[i].endpt->Peek ();
                DEBUG_PRINT ("peaking message %p", nextinmsgs[i]);
                inports[i].endpt->Pop ();
            }
        }
    }

    Message *outmsg = nullptr;
    operation ("select message")
    {
        uint64_t inputid = nextctrlmsgs[0].value;
        
        for (auto i = 0; i < ninports; i++)
        {
            Message *inmsg = inports[i].endpt->Peek ();

            if (i == inputid)
                outmsg = inmsg;
            else if (unlikely (inmsg != nullptr))
            {
                SIM_WARNING ("message dropped (portname: %s)",
                        GetFullName().c_str(), inports[i].name.c_str());
                inmsg->Dispose ();
            }
        }

        // NOTE: automatically broadcast to all post-mux modules
        if (outmsg)
            outmsg->DEST_RHS_ID = -1;
    }

    operation ("status check")
    {
        if (!outmsg)    cclass.idle++;
        else            cclass.active++;
    }

    operation ("assign to post-mux pathway LHS")
    {
        if (unlikely (Message::IsReserveMsg (outmsg)))
            SYSTEM_ERROR ("RESERVE message cannot reach this point (%s)",
                    GetFullName().c_str());

        bool assnres = outport.endpt->Assign (outmsg);
        if (unlikely (!assnres))
            SYSTEM_ERROR ("attempted to push to full LHS queue");
    }
}
