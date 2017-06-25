/***
 * TSim Testbench prototype (body)
 **/

#pragma once

#include <TSim/Prototype/ExampleTestbench.h>
#include <TSim/Prototype/ExampleModule.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


// (TODO) export your testbench to simulator
EXPORT_TESTBENCH (ExampleTestbench);


// (TODO) call base constructor with testbench name and top component (=TOP_COMPONENT)
ExampleTestbench::ExampleTestbench ()
    : Testbench ("ExampleTestbench", new ExampleComponent ("top", nullptr)) 
{
    #if 0
    // (TODO) cache the module for terminal condition check
    exmodule = static_cast<ExampleModule *>(TOP_MODULE->GetModule ("dummymodule2"));
    #endif
}

bool ExampleTestbench::IsFinished (PERMIT(Simulator))
{
    #if 0
    // (TODO) check terminal conditions
    return (exmodule->counter >= 10);
    #endif
}
