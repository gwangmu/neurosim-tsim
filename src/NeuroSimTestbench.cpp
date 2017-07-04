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


NeuroSimTestbench::NeuroSimTestbench ()
    : Testbench ("NeuroSimTestbench", new NeuroSim ("top", nullptr)) 
{
    datasink = dynamic_cast<DataSinkModule<AxonMessage, uint32_t> *>(TOP_COMPONENT->GetUnit ("datasink"));
    if (!datasink)
        SYSTEM_ERROR ("what the?");
}

bool NeuroSimTestbench::IsFinished (PERMIT(Simulator))
{
    DEBUG_PRINT ("%u", datasink->recvdata);
    return (datasink->recvdata >= 4);
}
