#include <TSim/Utility/AccessKey.h>

#include <Component/ExampleComponent.h>
#include <Component/NeuronBlock.h>
#include <NeuroSimTestbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;


EXPORT_TESTBENCH (NeuroSimTestbench);


NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new ExampleComponent ("top", nullptr)) 
{
    neuron_block = static_cast<NeuronBlock *>(TOP_COMPONENT->GetModule ("neuron_block"));
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    DEBUG_PRINT("GM BABO\n");
    return true;
}
