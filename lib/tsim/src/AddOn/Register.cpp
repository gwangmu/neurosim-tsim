#include <TSim/AddOn/Register.h>
#include <TSim/Utility/AccessKey.h>

#include <string>
#include <type_traits>

using namespace std;

struct RegisterWord;
class Module;


Register::Register (const char *clsname, string iname, 
        Type type, Attr attr, RegisterWord *wproto)
    : AddOn (clsname, iname), attr (attr)
{
    this->type = type;
    this->wproto = wproto;
}
