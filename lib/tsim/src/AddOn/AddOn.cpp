#include <TSim/AddOn/AddOn.h>
#include <TSim/Module/Module.h>

#include <string>

using namespace std;


string AddOn::GetFullName ()
{
    if (!parent)
        return GetName();
    else
        return string(GetClassName()) + " " + 
            parent->GetFullNameWOClass() + "::" + 
            GetInstanceName();
}
