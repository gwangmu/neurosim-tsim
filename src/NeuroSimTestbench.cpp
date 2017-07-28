#include <NeuroSimTestbench.h>

#include <TSim/Utility/Logging.h>
#include <TSim/Utility/AccessKey.h>

#include <Component/NeuroSimSystem.h>
#include <Component/TimestepReporter.h>
#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;

EXPORT_TESTBENCH (NeuroSimTestbench);
USING_TESTBENCH;

NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new LazyComponentCreator<NeuroSimSystem>()) {}

void NeuroSimTestbench::Initialize (PERMIT(Simulator))
{
    tsrep = dynamic_cast<TimestepReporter *>(TOP_COMPONENT->GetUnit("tsrep"));
    if (!tsrep) SYSTEM_ERROR ("wrong!");

    max_timestep = GET_PARAMETER(max_timestep);
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    return (tsrep->cur_timestep > max_timestep);
}
