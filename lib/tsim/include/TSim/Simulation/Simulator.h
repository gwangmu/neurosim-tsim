#pragma once        //< Why is this NOT a standard?

#include <TSim/Interface/IClockable.h>
#include <TSim/Utility/AccessKey.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Testbench;
class Component;
class Module;
class Device;
class Pathway;
class FileScript;
class FileRegister;


class Simulator final
{
public:
    struct Option
    {
        Option () {}
        uint64_t timelimit = -1;
        uint64_t tsinterval = -1;
        string gvfilename = "";
    };

protected:
    struct ClockDomain
    {
        ClockDomain ();

        // Clock properties
        string name;
        uint32_t period;    // unit: nanoseconds

        // Configuration
        vector<Module *> modules;
        vector<Device *> devices;
        vector<Pathway *> pathways;

        // Clock functions (in execution order)
        // HAVE FUN WITH FUNCTION POINTERS! XD
        class Clocker
        {
        public:
            Clocker (IClockable *iclk, IClockable::ClockFunction clkfn)
                : iclk (iclk), clkfn (clkfn) {}

            inline void Invoke (PERMIT(Simulator)) 
            { (iclk->*clkfn) (TRANSFER_KEY(Simulator)); }

        private:
            IClockable *iclk;
            IClockable::ClockFunction clkfn;
        };            
        vector<Clocker> clockers;

        // Simulation states
        uint64_t nexttime;
        uint64_t ncycles;
    };

public:
    Simulator (string specfilename, Option opt = Option());

    bool AttachTestbench (Testbench *tb);
    bool Simulate ();
    
    bool PrintGraphVizSource (string filename);
    void ReportDesignSummary ();
    void ReportSimulationSummary ();
    void ReportActivityEvents ();

private:
    void ReportComponentRec (Component *comp, uint32_t level);

    // 'AttachTestbench' subfunctions
    bool LoadTestbench ();
    bool ValidateTestbench ();

    // Simulator setting
    Option opt;

    // Loaded design
    string specfilename;
    Testbench *tb;
    vector<ClockDomain> cdomains;
    vector<FileScript *> fscrs;
    vector<FileRegister *> regs;

    // Simulation states
    uint64_t curtime;
    uint32_t runtime;
};


