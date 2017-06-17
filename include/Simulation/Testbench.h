
/* to be used in module declarations */
#define TESTBENCH_VISIBLE friend class Testbench

/* to be used at the bottom of testbench header */
#define EXPORT_TESTBENCH(TB) Testbench *simtb = new TB()

/* to be used in constructor */
#define TESTBENCH_NAME(name) clsname = name
#define SCRIPT_PATH(scr,path) fscrpaths[scr] = path
#define REGDATA_PATH(reg,path) regpaths[reg] = path
#define CLOCK_PERIOD(clk,period) clkperiods[clk] = period

class Testbench: public IMetadata
{
public:
    enum ParamType { FILESCRIPT_PATH, REGISTER_PATH, CLOCK_PERIOD };

pubilc:
    Testbench (string clsname, string name);

    virtual string GetName () final;
    virtual string GetFullName () final;
    virtual string GetClassName () final;
    virtual string GetSummary ();

    /* Called by 'Simulator' */
    Component* LoadTopComponent (PERMIT(Simulator)) = 0;
    string GetStringParam (ParamType ptype, string pname, PERMIT(Simulator));
    uint32_t GetUIntParam (ParamType ptype, string pname, PERMIT(Simulator));

    virtual bool IsFinished (PERMIT(Simulator)) = 0;

protected:
    string name;
    string clsname;

    map<string, string> fscrpaths;
    map<string, string> regpaths;
    map<string, uint32_t> clkperiods;
};
    
