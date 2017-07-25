#include <TSim/Register/Register.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <type_traits>

using namespace std;

struct RegisterWord;
class Module;


Register::Register (const char *clsname, Type type, Attr attr, RegisterWord *wproto)
    : Metadata (clsname, ""), attr (attr)
{
    this->type = type;
    this->wproto = wproto;
    this->parent = nullptr;

    rdenergy = wrenergy = -1;
    rdcount = wrcount = 0;
}

uint32_t Register::GetAccumReadEnergy ()
{
    if (rdenergy == -1) return -1;
    else return (rdenergy * rdcount);
}

uint32_t Register::GetAccumWriteEnergy ()
{
    if (wrenergy == -1) return -1;
    else return (wrenergy * wrcount);
}
