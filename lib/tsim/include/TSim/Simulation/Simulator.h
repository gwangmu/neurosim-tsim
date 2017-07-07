#pragma once        //< Why is this NOT a standard?

#include <TSim/Interface/IClockable.h>
#include <TSim/Base/Metadata.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <type_traits>
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
            template <
                typename T,
                typename = typename std::enable_if<
                    std::is_base_of<Metadata, T>::value &&
                    std::is_base_of<IClockable, T>::value
                >::type
            >
            Clocker (T *cls, IClockable::ClockFunction clkfn)
                : iclk (dynamic_cast<IClockable *>(cls)),
                meta (dynamic_cast<Metadata *>(cls)), clkfn (clkfn) {}

            // TODO: optimizable?
            inline void Invoke (PERMIT(Simulator)) 
            { (iclk->*clkfn) (TRANSFER_KEY(Simulator)); }
            
            inline string GetTagString ()
            { 
                string fntype = "";
                if (clkfn == &IClockable::PreClock)
                    fntype = "PreClock";
                else if (clkfn == &IClockable::PostClock)
                    fntype = "PostClock";
                else
                    SYSTEM_ERROR ("unknown clock function");

                return (meta ? meta->GetName() + "." + fntype : "");
            }

        private:
            IClockable *iclk;
            Metadata *meta;
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
    void ReportClockFunctionSchedule (ClockDomain &cdom);
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


