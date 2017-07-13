#include <TSim/Utility/Logging.h>
#include <TSim/Utility/AccessKey.h>

#include <Component/NeuroSim.h>
#include <NeuroSimTestbench.h>
#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;

EXPORT_TESTBENCH (NeuroSimTestbench);
USING_TESTBENCH;

NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new LazyComponentCreator<NeuroSim> ()) {}

void NeuroSimTestbench::Initialize (PERMIT(Simulator))
{
    ts_mgr = dynamic_cast<TSManager*> (TOP_COMPONENT->GetUnit("ts_manager"));
    if (!ts_mgr)
        SYSTEM_ERROR ("what the?");

    max_timestep = GET_PARAMETER(max_timestep);
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    return (ts_mgr->cur_timestep > max_timestep);
}
