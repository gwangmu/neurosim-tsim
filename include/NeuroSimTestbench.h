#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Component/NeuronBlock.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class NeuroSimTestbench: public Testbench
{
public:
    NeuroSimTestbench ();
    virtual bool IsFinished (PERMIT(Simulator));

private:
    NeuronBlock *neuron_block;
};
