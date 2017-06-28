#include <TSim/Utility/AccessKey.h>

#include <Component/ExampleComponent.h>
#include <Component/NeuroSim.h>
#include <NeuroSimTestbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;


EXPORT_TESTBENCH (NeuroSimTestbench);


NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new NeuroSim ("top", nullptr)) 
{
    //neurosim = static_cast<NeuroSim *>(TOP_COMPONENT->GetModule ("neurosim"));
    datasink = static_cast<DataSinkModule *>(TOP_COMPONENT->GetModule ("datasink"));
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    DEBUG_PRINT("GM BABO\n");
    return true;
}
