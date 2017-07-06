#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Message/AxonMessage.h>
#include <Component/DataSinkModule.h>
#include <Component/TSManager.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

struct NeuronBlockOutMessage;

class NeuroSimTestbench: public Testbench
{
public:
    NeuroSimTestbench ();
    virtual bool IsFinished (PERMIT(Simulator));

private:
    TSManager *ts_mgr;
    DataSinkModule<AxonMessage, uint32_t> *datasink;
};
