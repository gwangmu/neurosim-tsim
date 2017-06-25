#include <Pathway/Wire.h>
#include <Pathway/Message.h>
#include <Base/Component.h>
#include <Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


Wire::Wire (Component *parent, ConnectionAttr conattr, Message *msgproto)
    : Pathway ("Wire", parent, conattr, msgproto)
{
    AddEndpoint ("lhs", Endpoint::LHS, 0);
    AddEndpoint ("rhs", Endpoint::RHS, 0);
}

uint32_t Wire::TargetLHSEndpointID ()
{
    return 0;
}
