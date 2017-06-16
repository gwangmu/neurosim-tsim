
class Simulator
{
protected:
    struct ClockDomain
    {
        // Clock Properties
        string name;
        uint32_t period;    // unit: nanoseconds

        // Configuration
        vector<Module *> modules;

        // Simulation States
        uint64_t nexttime;
    };

public:
    // Configuration
    bool LoadDesign (Component *component);
    bool LoadScript (string modulename, string scrfile);
    bool LoadRegisterData (string modulename, string datafile);

    // Simulation
    bool Simulate (uint64_t interval);

    void Report ();

private:
    vector<ClockDomain> cdomains;

    // Simulation States
    uint32_t idxMinClock;
    uint64_t curtime;
};
