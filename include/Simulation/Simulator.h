#pragma once        //< Why is this NOT a standard?

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;

class Testbench;
class Module;
class Pathway;
class FileScript;
class Register;


class Simulator final
{
protected:
    struct ClockDomain
    {
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
    bool AttachTestbench (Testbench *tb);
    bool Simulate ();
    // void Report (); TODO

private:
    // 'AttachTestbench' subfunctions
    bool LoadTestbench ();
    bool ValidateTestbench ();

    // Loaded design
    Testbench *tb;
    vector<ClockDomain> cdomains;
    vector<FileScript *> fscrs;
    vector<Register *> regs;

    // Simulation states
    uint64_t curtime;
};


