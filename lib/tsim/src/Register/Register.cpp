#include <TSim/Register/Register.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Module/Module.h>

#include <string>
#include <type_traits>

using namespace std;

struct RegisterWord;


Register::Register (const char *clsname, Type type, Attr attr, RegisterWord *wproto)
    : Metadata (clsname, ""), attr (attr)
{
    this->type = type;
    this->wproto = wproto;
    this->parent = nullptr;

    rdenergy = wrenergy = -1;
    rdcount = wrcount = 0;
    stapower = -1;
}

double Register::GetConsumedStaticEnergy ()
{
    if (stapower == -1) return -1;
    else return (parent->GetClockPeriod() * 1E-9 * 
        stapower * 1E-9 * parent->GetTotalCycleCount());
}

double Register::GetAccumReadEnergy ()
{
    if (rdenergy == -1) return -1;
    else return (rdenergy * rdcount) * 1E-9;
}

double Register::GetAccumWriteEnergy ()
{
    if (wrenergy == -1) return -1;
    else return (wrenergy * wrcount) * 1E-9;
}
