#include <TSim/Device/Mux.h>

#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Simulation/Simulator.h>
#include <TSim/Utility/Prototype.h>
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
        CreatePort (string("input") + to_string(i), Device::PORT_INPUT, msgproto);

    // NOTE: ID should be 0
    CreatePort (string("select"), Device::PORT_CONTROL, Prototype<IntegerMessage>::Get());
}


void Mux::PreClock (PERMIT(Simulator))
{
    operation ("update control messages")
    {
        if (IntegerMessage *intmsg = 
                static_cast<IntegerMessage *>(ctrlports[0].endpt->Peek ()))
        {
            DEBUG_PRINT ("peak/pop ctrl port");
            nextctrlmsgs[0] = *intmsg;
            ctrlports[0].endpt->Pop ();
        }
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
                    (!outports[0].endpt->IsBroadcastBlocked ());
        }
    }
}

void Mux::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("calc '%s'", GetFullName().c_str());

    Message *outmsg = nullptr;
    operation ("select message")
    {
        uint64_t inputid = nextctrlmsgs[0].value;
        
        for (auto i = 0; i < ninports; i++)
        {
            Message *inmsg = inports[i].endpt->Peek ();

            if (inmsg)
            {
                inports[i].endpt->Pop ();
                if (i == inputid)
                    outmsg = inmsg;
                else if (unlikely (inmsg != nullptr))
                {
                    SIM_WARNING ("message dropped (portname: %s)",
                            GetFullName().c_str(), inports[i].name.c_str());
                    inmsg->Dispose ();
                }
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

    if (outmsg)
    {
        operation ("assign to post-mux pathway LHS")
        {
            if (unlikely (Message::IsReserveMsg (outmsg)))
                SYSTEM_ERROR ("RESERVE message cannot reach this point (%s)",
                        GetFullName().c_str());

            bool assnres = outports[0].endpt->Assign (outmsg);
            if (unlikely (!assnres))
                SYSTEM_ERROR ("attempted to push to full LHS queue");
        }
    }
}
