#include <TSim/Device/AndGate.h>

#include <TSim/Utility/Logging.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Message* AndGate::Logic (Message const * const *inmsgs)
{
    Message *retmsg = nullptr;
    IntegerMessage newoutmsg (1);
    
    for (auto i = 0; i < ninports; i++)
        newoutmsg.value &= (static_cast<IntegerMessage *>inmsgs[i])->value;

    if (newoutmsg.value != cached_output.value)
    {
        retmsg = new IntegerMessage (newoutmsg.value);
        cached_output.value = newoutmsg.value;
    }

    return retmsg;
}
