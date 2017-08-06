#include <TSim/Device/AndGate.h>

#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


Message* AndGate::Logic (Message const * const *inmsgs)
{
    Message *retmsg = nullptr;
    IntegerMessage newoutmsg (-1);
    
    for (auto i = 0; i < ninports; i++)
        newoutmsg.value &= static_cast<IntegerMessage const *>(inmsgs[i])->value;

    if (!cached || (newoutmsg.value != cached_output.value))
    {
        retmsg = new IntegerMessage (newoutmsg.value);
        DEBUG_PRINT ("[AND] sending %p %lu (before: %lu) (cached %d)", this, 
                newoutmsg.value, cached_output.value, cached);
        cached_output.value = newoutmsg.value;
        cached = true;
    }

    return retmsg;
}
