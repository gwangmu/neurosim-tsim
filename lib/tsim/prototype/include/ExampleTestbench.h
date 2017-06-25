/***
 * TSim Testbench prototype (header)
 **/

#pragma once

#include <TSim/Simulation/Testbench.h>
#include <Prototype/ExampleModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


// (TODO) replace 'ExampleTestbench' to the name you want to put
class ExampleTestbench: public Testbench
{
public:
    ExampleTestbench ();
    virtual bool IsFinished (PERMIT(Simulator));

private:
    #if 0
    // (TODO) declare cache variables
    ExampleModule *exmodule;
    #endif
};
