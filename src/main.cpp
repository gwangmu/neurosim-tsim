#include <Simulation/Simulator.h>
#include <Simulation/Testbench.h>

extern Testbench *simtb;

int main ()
{
    Simulator sim;

    if (!sim.AttachTestbench (simtb))
    {
        DESIGN_ERROR ("design error(s) detected. exiting.."
                simtb->GetClassName().c_str());
        abort ();
    }

    sim.Simulate ();
    // sim.Report (); TODO

    return 0;
}
