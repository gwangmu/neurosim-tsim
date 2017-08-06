#pragma once

#include <TSim/Simulation/Testbench.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class ExampleTestbench: public Testbench
{
public:
    ExampleTestbench ();
    virtual void Initialize (PERMIT(Simulator));
    virtual bool IsFinished (PERMIT(Simulator));
    virtual void Finalize (PERMIT(Simulator)) {}
};
