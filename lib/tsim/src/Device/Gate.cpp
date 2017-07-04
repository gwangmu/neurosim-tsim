#include <TSim/Device/Gate.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Gate::Gate (const char *clsname, string iname, Component *parent, 
        Message *msgproto, uint32_t ninput)
    : Device (clsname, iname, parent, msgproto)
{
    if (msgproto->GetType() != Message::TOGGLE)
        DESIGN_FATAL ("Gate only accepts TOGGLE type messages", GetName().c_str());

    // NOTE: IDs should be 0, 1, 2, ...
    for (int i = 0; i < ninput; i++)
        CreatePort (string("input") + to_string(i), Device::PORT_INPUT, msgproto);

    InitInputReadyState ();
}

void Gate::PostClock (PERMIT(Simulator))
{
    MICRODEBUG_PRINT ("calc '%s'", GetFullName().c_str());

    bool in_updated = false;
    operation ("update input")
    {
        for (auto i = 0; i < ninports; i++)
        {
            Message *inmsg = inports[i].endpt->Peek ();
            if (inmsg)
            {
                inports[i].endpt->Pop ();
                if (nextinmsgs[i]) nextinmsgs[i]->Dispose ();
                nextinmsgs[i] = inmsg;
                SetInputReadyState (i);
                in_updated = true;
            }
        }
    }

    if (IsInputReady ())
    {
        operation ("logic")
        {
            Message *outmsg = Logic (nextinmsgs);
            if (outmsg)
            {
                bool assnres = outports[0].endpt->Assign (outmsg);
                if (unlikely (!assnres))
                    SYSTEM_ERROR ("attempted to push to full LHS queue");
            }
        }
    }

    operation ("status check")
    {
        if (in_updated) cclass.idle++;
        else            cclass.active++;
    }
}
