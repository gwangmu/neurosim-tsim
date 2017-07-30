#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Message/AxonMessage.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class TimestepReporter;

class NeuroSimTestbench: public Testbench
{
public:
    NeuroSimTestbench ();
    virtual void Initialize (PERMIT(Simulator));
    virtual bool IsFinished (PERMIT(Simulator));
    virtual void Finalize (PERMIT(Simulator));

private:
    TimestepReporter *tsrep;
    int max_timestep;
};
