#include <TSim/Utility/AccessKey.h>

#include <Component/NeuroSim.h>
#include <NeuroSimTestbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Simulator;


//EXPORT_TESTBENCH (NeuroSimTestbench);


NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new NeuroSim ("top", nullptr)) 
{
    //datasink = static_cast<DataSinkModule *>(TOP_COMPONENT->GetModule ("datasink"));
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    return true;
    //return (datasink->recvdata >= 50);
}
