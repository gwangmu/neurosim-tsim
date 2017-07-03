#pragma once        //< Why is this NOT a standard?

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
        vector<Pathway *> pathways_postdev;
        vector<Pathway *> pathways_ctrl;

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


