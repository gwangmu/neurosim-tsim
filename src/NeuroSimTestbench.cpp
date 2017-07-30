#include <NeuroSimTestbench.h>

#include <TSim/Utility/Logging.h>
#include <TSim/Utility/AccessKey.h>

#include <Component/NeuroSimSystem.h>
#include <Component/PseudoStorage.h>
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
    tsrep = dynamic_cast<TimestepReporter *>(TOP_COMPONENT->GetUnit("top.tsrep"));
    if (!tsrep) SYSTEM_ERROR ("wrong!");

    max_timestep = GET_PARAMETER(max_timestep);
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    return (tsrep->cur_timestep > max_timestep);
}

void NeuroSimTestbench::Finalize (PERMIT(Simulator))
{
    //int num_propagators = GET_PARAMETER(num_propagators);
    int num_propagators = 1;

    for(int i=0; i<num_propagators; i++)
    {
        auto axon_storage = 
            dynamic_cast<PseudoStorage*>(
                    TOP_COMPONENT->GetUnit("top.neurosim0.propagator" +
                                           to_string(i) +
                                           ".axon_storage"));

        axon_storage->PrintStats();
    }

    return;
}
