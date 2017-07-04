#include <TSim/Utility/AccessKey.h>

#include <Component/NeuroSim.h>
#include <NeuroSimTestbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;
struct NeuronBlockOutMessage;

EXPORT_TESTBENCH (NeuroSimTestbench);


NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new NeuroSim ("top", nullptr)) 
{
    //datasink = static_cast<DataSinkModule<NeuronBlockOutMessage, uint32_t> *>(TOP_COMPONENT->GetModule ("datasink"));
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    return 0; //(datasink->recvdata >= 4);
}
