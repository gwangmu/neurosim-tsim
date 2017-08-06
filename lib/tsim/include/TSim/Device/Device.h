#pragma once

#include <TSim/Base/Unit.h>
#include <TSim/Pathway/IntegerMessage.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <set>

using namespace std;

namespace TSim
{
    class Module;
    class Endpoint;
    class Simulator;
    class Message;

    class Device: public Unit 
    {
    public:
        static const char *const OUTPORT_NAME;
    
    public:
        /* Universal */
        Device (const char* clsname, string iname, Component *parent, Message *msgproto);
        virtual string GetClock (); 
        std::set <string> GetClockSet();
    
        /* Called by 'Simulator' */
        virtual IssueCount Validate (PERMIT(Simulator));
        virtual void PreClock (PERMIT(Simulator)) = 0;
        virtual void PostClock (PERMIT(Simulator)) = 0;
        
        /* Called by 'Component' */
        virtual bool IsValidConnection (Port *port, Endpoint *endpt);
    
    protected:  
        /* Called by derived 'Module' */
        virtual void OnCreatePort (Port &newport);
    
        Message **nextinmsgs;
        IntegerMessage *nextctrlmsgs;
    
    private:
        // port management
        Message* msgproto;
    };
}
