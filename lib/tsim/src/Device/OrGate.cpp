#include <TSim/Device/OrGate.h>

#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Message* OrGate::Logic (Message const * const *inmsgs)
{
    Message *retmsg = nullptr;
    IntegerMessage newoutmsg (0);
    
    for (auto i = 0; i < ninports; i++)
        newoutmsg.value |= static_cast<IntegerMessage const *>(inmsgs[i])->value;

    if (!cached || newoutmsg.value != cached_output.value)
    {
        retmsg = new IntegerMessage (newoutmsg.value);
        cached_output.value = newoutmsg.value;
        DEBUG_PRINT ("sending %lu", cached_output.value);
        cached = true;
    }

    return retmsg;
}
