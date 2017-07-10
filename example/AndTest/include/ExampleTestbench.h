#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Component/DataSinkModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class ExampleTestbench: public Testbench
{
public:
    ExampleTestbench ();
    virtual void Initialize (PERMIT(Simulator));
    virtual bool IsFinished (PERMIT(Simulator));

private:
    DataSinkModule *datasink;
};
