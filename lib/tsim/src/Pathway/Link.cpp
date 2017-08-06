#include <TSim/Pathway/Link.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Base/Component.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


Link::Link (Component *parent, ConnectionAttr conattr, double link_speed,
        Message *msgproto)
    : Pathway ("Link", parent, conattr, msgproto)
{
    AddEndpoint ("lhs", Endpoint::LHS, 0);
    AddEndpoint ("rhs", Endpoint::RHS, 1);

    bitwidth = conattr.bitwidth;
    this->link_speed = link_speed;
    factor = -1;
}

void Link::ApplyReferenceClockPeriod (uint32_t period)
{
    factor = link_speed / (1 / TO_SPEC_TIMEUNIT(period));
    SetEffectiveBitWidth (bitwidth * factor);
}

uint32_t Link::GetBitWidth ()
{
    if (factor != -1)
        return GetEffectiveBitWidth() / factor;
    else
        return GetEffectiveBitWidth();
}

uint32_t Link::NextTargetLHSEndpointID ()
{
    return 0;
}
