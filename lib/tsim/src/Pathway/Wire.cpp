#include <TSim/Pathway/Wire.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Base/Component.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


Wire::Wire (Component *parent, ConnectionAttr conattr, Message *msgproto)
    : Pathway ("Wire", parent, conattr, msgproto)
{
    AddEndpoint ("lhs", Endpoint::LHS, 0);
    AddEndpoint ("rhs", Endpoint::RHS, 1);
}

uint32_t Wire::NextTargetLHSEndpointID ()
{
    return 0;
}
