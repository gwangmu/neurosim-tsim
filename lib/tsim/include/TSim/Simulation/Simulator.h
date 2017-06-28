#pragma once        //< Why is this NOT a standard?

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Testbench;
class Module;
class Pathway;
class AddOn;
class ILoadable;

class Simulator final
{
public:
    struct Option
    {
        Option () {}
        uint64_t timelimit = -1;
        uint64_t tsinterval = -1;
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
        vector<Pathway *> pathways;

        // Simulation states
        uint64_t nexttime;
    };

public:
    Simulator (string specfilename, Option opt = Option());

    bool AttachTestbench (Testbench *tb);
    bool Simulate ();
    // void Report (); TODO

private:
    // 'AttachTestbench' subfunctions
    bool LoadTestbench ();
    bool ValidateTestbench ();

    // Simulator setting
    Option opt;

    // Loaded design
    string specfilename;
    Testbench *tb;
    vector<ClockDomain> cdomains;
    vector<AddOn *> ld_addons;

    // Simulation states
    uint64_t curtime;
};


