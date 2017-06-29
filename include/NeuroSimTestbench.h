#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Component/DataSinkModule.h>

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
    DataSinkModule<NeuronBlockOutMessage, uint32_t> *datasink;
};
