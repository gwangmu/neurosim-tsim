#include <TSim/Utility/AccessKey.h>

#include <Component/ExampleComponent.h>
#include <Component/DataSinkModule.h>
#include <ExampleTestbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;


EXPORT_TESTBENCH (ExampleTestbench);

ExampleTestbench::ExampleTestbench ()
    : Testbench ("ExampleTestbench", new LazyComponentCreator<ExampleComponent> ()) {}

void ExampleTestbench::Initialize (PERMIT(Simulator))
{
    datasink = dynamic_cast<DataSinkModule *>(TOP_COMPONENT->GetUnit ("datasink"));
    if (!datasink)
        SYSTEM_ERROR ("something is wrong!");
}

bool ExampleTestbench::IsFinished (PERMIT(Simulator))
{
    DEBUG_PRINT ("%u", datasink->recvdata);
    return (datasink->recvdata >= 50);
}
