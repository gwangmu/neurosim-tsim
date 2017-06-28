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
    : Testbench ("ExampleTestbench", new ExampleComponent ("top", nullptr)) 
{
    datasink = static_cast<DataSinkModule *>(TOP_COMPONENT->GetModule ("datasink"));
}

bool ExampleTestbench::IsFinished (PERMIT(Simulator))
{
    return (datasink->recvdata >= 50);
}
