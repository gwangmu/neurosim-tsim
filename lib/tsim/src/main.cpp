#include <TSim/Simulation/Simulator.h>
#include <TSim/Simulation/Testbench.h>
#include <TSim/Utility/Logging.h>

#include <string>

using namespace std;

extern Testbench *simtb;

int main (int argc, char *argv[])
{
    if (argc < 2) {
        PRINT ("usage: %s <simspec_file>", argv[0]);
        return 0;
    }
                
    Simulator sim (argv[1]);

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
