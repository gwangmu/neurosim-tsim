#include <TSim/Utility/AccessKey.h>

#include <Component/ExampleComponent.h>
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
}

bool ExampleTestbench::IsFinished (PERMIT(Simulator))
{
    return false;
}
