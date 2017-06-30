#include <TSim/Pathway/RRFaninWire.h>
#include <TSim/Pathway/Message.h>
#include <TSim/Base/Component.h>
#include <TSim/Utility/Logging.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


RRFaninWire::RRFaninWire (Component *parent, ConnectionAttr conattr, 
        Message *msgproto, uint32_t n_lhs)
    : Pathway ("RRFaninWire", parent, conattr, msgproto)
{
    if (n_lhs == 0)
        DESIGN_FATAL ("#lhs cannot be zero", GetName().c_str());
    else if (n_lhs == 1)
        DESIGN_WARNING ("'Wire' is recommended for #lhs=1 case", GetName().c_str());

    for (uint32_t i = 0; i < n_lhs; i++)
        AddEndpoint ("lhs" + to_string(i), Endpoint::LHS, 0);

    AddEndpoint ("rhs", Endpoint::RHS, 1);

    cur_lhsid = 0;
}

uint32_t RRFaninWire::NextTargetLHSEndpointID () 
{
    uint32_t next_lhsid = cur_lhsid;
    do
    {
        next_lhsid = cur_lhsid + 1;
        if (next_lhsid >= GetNumLHS ())
            next_lhsid = 0;

        // select next non-empty LHS
        // NOTE: lhs whose capacity==0 is always taken
        Endpoint &tar_lhs = GetLHS (next_lhsid);
        if (tar_lhs.GetCapacity() == 0 || !tar_lhs.IsEmpty ())
        {
            cur_lhsid = next_lhsid;
            break;
        }

    } while (next_lhsid != cur_lhsid);

    return cur_lhsid;
}
