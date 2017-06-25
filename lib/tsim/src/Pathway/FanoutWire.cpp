#include <TSim/Pathway/FanoutWire.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Base/Component.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


FanoutWire::FanoutWire (Component *parent, ConnectionAttr conattr, 
        Message *msgproto, uint32_t n_rhs)
    : Pathway ("FanoutWire", parent, conattr, msgproto)
{
    AddEndpoint ("lhs", Endpoint::LHS, 1);

    if (n_rhs == 0)
        DESIGN_FATAL ("#rhs cannot be zero", GetName().c_str());
    else if (n_rhs == 1)
        DESIGN_WARNING ("'Wire' is recommended for #rhs=1 case", GetName().c_str());

    for (uint32_t i = 0; i < n_rhs; i++)
        AddEndpoint ("rhs" + to_string(i), Endpoint::RHS, 1);
}

uint32_t FanoutWire::TargetLHSEndpointID () 
{
    return 0; 
}
