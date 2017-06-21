#include <Simulation/Simulator.h>
#include <Simulation/Testbench.h>
#include <Utility/Logging.h>

#include <string>

using namespace std;

extern Testbench *simtb;

int main ()
{
    Simulator sim;

    if (!sim.AttachTestbench (simtb))
    {
        DESIGN_ERROR ("design error(s) detected. exiting..",
                simtb->GetName().c_str());
        abort ();
    }

    sim.Simulate ();
    // sim.Report (); TODO

    return 0;
}
