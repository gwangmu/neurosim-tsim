#include <TSim/Simulation/Simulator.h>

#include <TSim/Simulation/Testbench.h>
#include <TSim/Base/Unit.h>
#include <TSim/Module/Module.h>
#include <TSim/Module/PCIeSwitch.h>
#include <TSim/Device/Device.h>
#include <TSim/Device/Gate.h>
#include <TSim/Register/FileRegister.h>
#include <TSim/Script/FileScript.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Pathway/Link.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <chrono>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <set>

using namespace std;

// NOTE: CLOCK_PERIOD is written in NS (simspec), calculated in PS (simulation)

/* Constructors */
Simulator::ClockDomain::ClockDomain ()
{
    name = "";
    period = 0;
    nexttime = 0;
    ncycles = 0;
}


Simulator::Simulator (string specfilename, Simulator::Option opt)
{
    this->specfilename = specfilename;
    this->opt = opt;
    tb = NULL;
    curtime = 0;
}


/* function AttachTestbench */
bool Simulator::AttachTestbench (Testbench *tb)
{
    this->tb = tb;

    if (!LoadTestbench ())
        return false;

    if (!ValidateTestbench ())
        return false;

    return true;
}

bool Simulator::LoadTestbench ()
{
    macrotask ("Loading '%s'..", tb->GetName().c_str());

    task ("print graphviz design source")
    {
        if (opt.gvfilename != "")
            PrintGraphVizSource (opt.gvfilename);
    }

    task ("load simulation spec")
    {
        if (!tb->LoadSimulationSpec (specfilename, KEY(Simulator)))
            DESIGN_FATAL ("cannot load simulation spec '%s'",
                    tb->GetName().c_str(), specfilename.c_str());
    }

    task ("init testbench")
    {
        tb->CreateComponentAndInitialize (KEY(Simulator));
    }

    vector<Module *> modules;
    vector<Device *> devices;
    vector<Pathway *> pathways;
    uint32_t nmodules = 0;
    uint32_t npswitches = 0;
    uint32_t ndevices = 0;
    uint32_t ngates = 0;
    uint32_t npathways = 0;
    task ("find modules/pathways")
    {
        queue<Component *> queComps;
        queComps.push (tb->GetTopComponent (KEY(Simulator)));
        
        while (!queComps.empty ())
        {
            Component *nextcomp = queComps.front ();
            queComps.pop ();

            for (auto ipath = nextcomp->PathwayBegin ();
                    ipath != nextcomp->PathwayEnd (); ipath++)
            {
                Pathway *pathway = *ipath;
                pathways.push_back (pathway);
                npathways++;
            }

            for (auto icomp = nextcomp->ChildBegin ();
                    icomp != nextcomp->ChildEnd (); icomp++)
            {
                Component *comp = *icomp;

                if (Unit *unit = dynamic_cast<Unit *>(comp))
                {
                    if (Module *module = dynamic_cast<Module *>(unit))
                    {
                        modules.push_back (module);
                        nmodules++;

                        if (PCIeSwitch *pswitch = dynamic_cast<PCIeSwitch *>(module))
                        {
                            pswitches.push_back (pswitch);
                            npswitches++;
                        }
                    }
                    else if (Device *device = dynamic_cast<Device *>(unit))
                    {
                        devices.push_back (device);
                        ndevices++;
                        if (dynamic_cast<Gate *>(unit))
                            ngates++;
                    }
                    else
                        SYSTEM_ERROR ("bogus unit class");
                }
                else
                    queComps.push (comp);
            }
        }

        PRINT ("total %u module(s), %u device(s) (with %u gate(s)), %u pathway(s) found",
                nmodules, ndevices, ngates, npathways);
    }

    task ("init this->cdomains")
    {
        map<string, ClockDomain> mapCDoms;

        for (Module *module : modules)
        {
            string nclock = module->GetClock ();
            if (nclock == "")
                DESIGN_FATAL ("undefined module clock (module: %s)",
                        tb->GetName().c_str(), module->GetName().c_str());

            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].modules.push_back (module);

            module->SetDynamicPower 
                (tb->GetUIntParam (Testbench::UNIT_DYNAMIC_POWER, 
                                   module->GetClassName()),
                 KEY(Simulator));
            module->SetStaticPower 
                (tb->GetUIntParam (Testbench::UNIT_STATIC_POWER, 
                                   module->GetClassName()),
                 KEY(Simulator));
        }

        for (Device *device : devices)
        {
            set<string> clockset = device->GetClockSet(); 

            if (clockset.empty())
                DESIGN_FATAL ("undefined device clock (device: %s)",
                        tb->GetName().c_str(), device->GetName().c_str());

            for (auto& nclock: clockset)
            {
                mapCDoms[nclock].name = nclock;
                mapCDoms[nclock].devices.push_back (device);
            }

            device->SetDynamicPower 
                (tb->GetUIntParam (Testbench::UNIT_DYNAMIC_POWER, 
                                   device->GetClassName()),
                 KEY(Simulator));
            device->SetStaticPower 
                (tb->GetUIntParam (Testbench::UNIT_STATIC_POWER, 
                                   device->GetClassName()),
                 KEY(Simulator));
        }

        for (Pathway *pathway : pathways)
        {
            set<string> clockset = pathway->GetClockSet(); 

            if (clockset.empty())
                DESIGN_FATAL ("undefined pathway clock (pathway: %s)",
                        tb->GetName().c_str(), pathway->GetName().c_str());

            for (auto& nclock: clockset)
            {
                mapCDoms[nclock].name = nclock;
                mapCDoms[nclock].pathways.push_back (pathway);
            }

            pathway->SetDissipationPower 
                (tb->GetUIntParam (Testbench::PATHWAY_DIS_POWER, 
                                   pathway->GetClassName()),
                 KEY(Simulator));
        }
        
        for (auto &centry : mapCDoms)
            cdomains.push_back (centry.second);

        for (ClockDomain &cdomain : cdomains)
        {
            cdomain.period = tb->GetUIntParam (Testbench::CLOCK_PERIOD, cdomain.name);

            for (Module *module : cdomain.modules)
                module->SetClockPeriod (cdomain.period, KEY(Simulator));

            for (Device *device : cdomain.devices)
                device->SetClockPeriod (cdomain.period, KEY(Simulator));

            for (Pathway *pathway : cdomain.pathways)
            {
                pathway->SetClockPeriod (cdomain.period, KEY(Simulator));

                if (Link *link = dynamic_cast<Link *>(pathway))
                    link->ApplyReferenceClockPeriod (cdomain.period);
            }
        }

        PRINT ("total %zu clock domain(s) formed", cdomains.size());
    }

    task ("schedule intra-domain clock functions")
    {
        struct PathwayResolveState
        {
            PathwayResolveState (uint32_t nlhs, uint32_t nrhs)
            {
                lhs_msg = (uint64_t)-1 << nlhs;
                rhs_block = (uint64_t)-1 << nrhs;
            }

            uint64_t rhs_block = 0;
            uint64_t lhs_msg = 0;

            void ResolveRHSBlock (uint32_t id) { rhs_block |= (1 << id); }
            void ResolveLHSMsg (uint32_t id) { lhs_msg |= (1 << id); }
            bool IsResolved () { return (rhs_block == (uint64_t)-1) && (lhs_msg == (uint64_t)-1); }
        };

        struct DeviceResolveState
        {
            DeviceResolveState (uint32_t nin, uint32_t nctrl)
            {
                in_msg = (uint64_t)-1 << nin;
                ctrl_msg = (uint64_t)-1 << nctrl;
            }

            uint64_t ctrl_msg = 0;
            bool ctrl_sched = false;
            uint64_t in_msg = 0;

            void SetNumCtrl (uint32_t n) { ctrl_msg = (uint64_t)-1 << n; }
            void SetNumInput (uint32_t n) { in_msg = (uint64_t)-1 << n; }
            void ResolveCtrlMsg (uint32_t id) { ctrl_msg |= (1 << id); }
            void ResolveInputMsg (uint32_t id) { in_msg |= (1 << id); }
            bool IsCtrlResolved () { return (ctrl_msg == (uint64_t)-1); }
            void ScheduleCtrl () { ctrl_sched = true; }
            bool IsCtrlScheduled () { return ctrl_sched; }
            bool IsInputResolved () { return (in_msg == (uint64_t)-1); }
        };

        for (ClockDomain &cdom : cdomains)
        {
            task ("push base clock functions")
            {
                for (Module *module : cdom.modules)
                    cdom.clockers.push_back (ClockDomain::Clocker (module, &IClockable::PreClock));

                for (Pathway *pathway : cdom.pathways)
                    if (pathway->IsPreModulePathway ())
                        cdom.clockers.push_back (ClockDomain::Clocker (pathway, &IClockable::PreClock));

                for (Module *module : cdom.modules)
                    cdom.clockers.push_back (ClockDomain::Clocker (module, &IClockable::PostClock));
            }

            map<Pathway *, PathwayResolveState> prstates;
            map<Device *, DeviceResolveState> drstates;

            task ("init pathway resolve states")
            {
                for (Pathway *pathway : cdom.pathways)
                {
                    prstates.insert (make_pair (pathway, 
                                PathwayResolveState (pathway->GetNumLHS(),
                                    pathway->GetNumRHS())));
                    
                    for (auto i = 0; i < pathway->GetNumLHS(); i++)
                    {
                        // NOTE: resolved by base clock functions
                        if (dynamic_cast<Module *>(pathway->GetLHS(i).GetConnectedUnit()))
                            prstates.find(pathway)->second.ResolveLHSMsg (i);

                        // NOTE: resolved if LHS unit not in this domain
                        if (Device *device = dynamic_cast<Device *>(pathway->GetLHS(i).GetConnectedUnit()))
                        {
                            if (find (cdom.devices.begin(), cdom.devices.end(), device) == cdom.devices.end())
                                prstates.find(pathway)->second.ResolveLHSMsg (i);
                        }
                    }

                    for (auto i = 0; i < pathway->GetNumRHS(); i++)
                    {
                        // NOTE: resolved by base clock functions
                        // NOTE: LHS blocking was determined at the previous clock
                        if (dynamic_cast<Module *>(pathway->GetRHS(i).GetConnectedUnit()))
                            prstates.find(pathway)->second.ResolveRHSBlock (i);

                        // NOTE: resolved if RHS unit not in this domain
                        if (Device *device = dynamic_cast<Device *>(pathway->GetRHS(i).GetConnectedUnit()))
                        {
                            if (find (cdom.devices.begin(), cdom.devices.end(), device) == cdom.devices.end())
                                prstates.find(pathway)->second.ResolveRHSBlock (i);
                        }
                    }

                    if (pathway->IsControlPathway ())
                    {
                        // NOTE: CONTROL pathways are never be blocked,
                        //  as message prototype must be TOGGLE-type
                        for (auto i = 0; i < pathway->GetNumRHS(); i++)
                            prstates.find(pathway)->second.ResolveRHSBlock (i);
                    }
                }

                for (Device *device : cdom.devices)
                {
                    drstates.insert (make_pair (device, 
                                DeviceResolveState (device->GetNumInPorts(),
                                    device->GetNumCtrlPorts())));

                    for (auto i = 0; i < device->GetNumInPorts(); i++)
                    {
                        Pathway *pathway = device->GetInEndpoint(i)->GetParent();

                        // NOTE: resolved if INPUT path not in this domain
                        if (find (cdom.pathways.begin(), cdom.pathways.end(), pathway) == cdom.pathways.end())
                            drstates.find(device)->second.ResolveInputMsg (i);
                    }

                    for (auto i = 0; i < device->GetNumCtrlPorts(); i++)
                    {
                        Pathway *pathway = device->GetCtrlEndpoint(i)->GetParent();

                        // NOTE: resolved if CONTROL path not in this domain
                        if (find (cdom.pathways.begin(), cdom.pathways.end(), pathway) == cdom.pathways.end())
                            drstates.find(device)->second.ResolveCtrlMsg (i);
                    }
                }
            }

            task ("schedule rest of clock functions")
            {
                while (!prstates.empty ())
                {
                    bool sched_something = false;

                    for (auto &entry : prstates)
                    {
                        Pathway *pathway = entry.first;
                        PathwayResolveState &prstate = entry.second;

                        if (prstate.IsResolved ())
                        {
                            operation ("schedule resolved pathways")
                            {
                                cdom.clockers.push_back (ClockDomain::Clocker (pathway, &IClockable::PostClock));
                                sched_something = true;
                            }

                            operation ("resolve devices")
                            {
                                for (auto i = 0; i < pathway->GetNumRHS(); i++)
                                {
                                    Endpoint &endpt = pathway->GetRHS(i);
                                    if (Device *postdev = dynamic_cast<Device *>(endpt.GetConnectedUnit ()))
                                    {
                                        string pname = endpt.GetConnectedPortName ();
                                        if (postdev->IsControlPort (pname))
                                            drstates.find(postdev)->second.ResolveCtrlMsg (postdev->GetCtrlPortID (pname));
                                        else if (postdev->IsInputPort (pname))
                                            drstates.find(postdev)->second.ResolveInputMsg (postdev->GetInPortID (pname));
                                        else
                                            SYSTEM_ERROR ("RHS cannot be connected to ports other than INPUT/CONTROL type");
                                    }
                                }
                            }

                            prstates.erase (pathway);
                        }
                    }

                    for (auto &entry : drstates)
                    {
                        Device *device = entry.first;
                        DeviceResolveState &drstate = entry.second;
                        
                        if (drstate.IsCtrlResolved ())
                        {
                            operation ("schedule ctrl-resolved devices")
                            {
                                if (!drstate.IsCtrlScheduled ())
                                {
                                    cdom.clockers.push_back (ClockDomain::Clocker (device, &IClockable::PreClock));
                                    drstate.ScheduleCtrl ();
                                    sched_something = true;
                                }
                            }

                            operation ("resolve pathways' rhs_block")
                            {
                                for (auto i = 0; i < device->GetNumInPorts(); i++)
                                {
                                    Endpoint *inept = device->GetInEndpoint (i);
                                    Pathway *prepath = inept->GetParent ();
                                    prstates.find(prepath)->second.ResolveRHSBlock (inept->GetID ());
                                }
                            }
                        }

                        if (drstate.IsInputResolved ())
                        {
                            operation ("schedule in-resolved devices")
                            {
                                if (!drstate.IsCtrlResolved ())
                                    SYSTEM_ERROR ("INPUT cannot be resolved before CONTROL resolved");

                                cdom.clockers.push_back (ClockDomain::Clocker (device, &IClockable::PostClock));
                                sched_something = true;
                            }

                            operation ("resolve pathways' lhs_msg")
                            {
                                for (auto i = 0; i < device->GetNumOutPorts(); i++)
                                {
                                    Endpoint *outept = device->GetOutEndpoint (i);
                                    Pathway *postpath = outept->GetParent ();
                                    prstates.find(postpath)->second.ResolveLHSMsg (outept->GetID ());
                                }
                            }

                            drstates.erase (device);
                        }                    
                    }

                    if (!sched_something)
                        DESIGN_FATAL ("cannot proceed clock function scheduling. (possible device loop)",
                                tb->GetName().c_str());
                }

                if (!drstates.empty ())
                    SYSTEM_ERROR ("empty 'prstates' must imply empty 'drstate'");
            }
        }

        for (ClockDomain &cdom : cdomains)
        {
            PRINT ("");
            ReportClockFunctionSchedule (cdom);
        }
        PRINT ("");
    }

    task ("init this->fscrs, this->regs")
    {
        for (Module *module : modules)
        {
            if (FileScript *fscr = dynamic_cast<FileScript *>(module->GetScript ()))
                fscrs.push_back (fscr);

            if (FileRegister *reg = dynamic_cast<FileRegister *>(module->GetRegister ()))
                regs.push_back (reg);
        }

        PRINT ("total %zu file script(s), %zu file register(s) found", 
                fscrs.size(), regs.size());

        task ("load file scripts") 
        {
            for (FileScript *fscr : fscrs)
            {
                string pname = fscr->GetParent()->GetFullNameWOClass();
                string path = tb->GetStringParam (Testbench::FILESCRIPT_PATH, pname);

                if (path != "")
                {
                    PRINT ("Loading '%s' <-- '%s'..", 
                            fscr->GetParent()->GetName().c_str(), path.c_str());
                    fscr->LoadScriptFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s' specified",
                            tb->GetName().c_str(),
                            fscr->GetName().c_str());
            }
        }

        task ("load register data") 
        {
            for (FileRegister *reg : regs)
            {
                string pname = reg->GetParent()->GetFullNameWOClass();
                string path = tb->GetStringParam (Testbench::REGISTER_DATAPATH, pname);

                if (path != "")
                {
                    PRINT ("Loading '%s' <-- '%s'..", 
                            reg->GetParent()->GetName().c_str(), path.c_str());
                    reg->LoadDataFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s' specified",
                            tb->GetName().c_str(), 
                            reg->GetName().c_str());
            }
        }
    }

    return true;
}

bool Simulator::ValidateTestbench ()
{
    macrotask ("Checking testbench validity..");

    task ("validate design")
    {
        IssueCount icount = tb->GetTopComponent(KEY(Simulator))->
            Validate(KEY(Simulator));

        for (FileScript *fscr : fscrs)
        {
            IssueCount ficount = fscr->Validate (KEY(Simulator));
            icount.error += ficount.error;
            icount.warning += ficount.warning;
        }

        for (FileRegister *reg : regs)
        {
            IssueCount ricount = reg->Validate (KEY(Simulator));
            icount.error += ricount.error;
            icount.warning += ricount.warning;
        }

        PRINT ("%d design error(s) and %d design warning(s)",
                icount.error, icount.warning);

        if (icount.error != 0)
            return false;
    }
    
    task ("vaildate clocks")
    {
        uint32_t nerr = 0;
        if (cdomains.empty ())
        {
            DESIGN_ERROR ("no clock found", tb->GetName().c_str());
            nerr++;
        }

        for (ClockDomain &cdom : cdomains)
        {
            if (cdom.period == 0)
            {
                DESIGN_ERROR ("clock '%s' has 0 period",
                        tb->GetName().c_str(), cdom.name.c_str());
                nerr++;
            }

            if (cdom.modules.empty())
                DESIGN_FATAL ("clock domain '%s' does not contain any module", 
                        tb->GetName().c_str(), cdom.name.c_str());
        }

        PRINT ("%d clock error(s)", nerr);
        
        if (nerr)
            return false;
    }

    return true;
}


/* function Simulate */
bool Simulator::Simulate ()
{
    auto start = chrono::steady_clock::now ();

    macrotask ("Initializing clocks..")
    {
        // offset clock start time by 1ns,
        // making 0th clock the first one.
        for (auto i = 0; i < cdomains.size(); i++)
            cdomains[i].nexttime = i;
    }

    macrotask ("Starting simulation..")
    {
        uint64_t nexttstime = 0;
        while (!tb->IsFinished (KEY(Simulator)))
        {
            ClockDomain *curCDom;
            operation ("advance clock")
            {
                auto minidx = -1;
                uint64_t mintime = (uint64_t)-1;
                for (auto i = 0; i < cdomains.size(); i++)
                {
                    if (cdomains[i].nexttime < mintime)
                    {
                        minidx = i;
                        mintime = cdomains[i].nexttime;
                    }
                }

                curtime = mintime;
                curCDom = &cdomains[minidx];
                cdomains[minidx].nexttime += cdomains[minidx].period;
                cdomains[minidx].ncycles++;
            }

            if (nexttstime <= curtime)
            {
                INFO_PRINT ("Simulating %.3lf ns..", TO_SPEC_TIMEUNIT(curtime));
                nexttstime += opt.tsinterval;
            }

            task ("simulate %.3lf ns", TO_SPEC_TIMEUNIT(curtime))
            {
                for (ClockDomain::Clocker &clocker : curCDom->clockers)
                    clocker.Invoke (KEY(Simulator));
            }

            if (curtime > opt.timelimit) 
            {
                PRINT ("Simulation reached time limit (%.3lf ns)", 
                        TO_SPEC_TIMEUNIT(opt.timelimit));
                break;
            }
        }
    }

    auto end = chrono::steady_clock::now ();
    chrono::duration<double> diff = end-start;
    runtime = diff.count();
    //runtime = chrono::duration_cast<chrono::milliseconds>(end - start).count ();

    PRINT ("Simulation finished at %.3lf ns", TO_SPEC_TIMEUNIT(curtime));

    return true;
}


bool Simulator::PrintGraphVizSource (string filename)
{
    string body;
    body = tb->GetTopComponent(KEY(Simulator))->GetGraphVizBody(1);

    string src = "";
    src += string ("digraph ") + string(tb->GetClassName()) + " {\n";
    src += "\tnode [shape = record];\n";
    src += "\tgraph [rankdir = LR];\n";
    src += body;
    src += string ("}");

    ofstream file (filename);
    if (!file.is_open ())
    {
        DESIGN_WARNING ("cannot create graphviz file '%s'",
                tb->GetClassName(), filename.c_str());
        return false;
    }

    file << src;
    file.close ();

    return true;
}

void Simulator::ReportDesignSummary ()
{
    macrotask ("< Design summary >");

#define STROKE PRINT ("%s", string(46, '-').c_str())
#define ROW(f, v) PRINT (" %-29s %14s ", f, v)

    STROKE;
    ROW ("Specification", "Value");
    STROKE;

    uint32_t nmodules = 0;
    for (ClockDomain &cdom : cdomains)
        nmodules += cdom.modules.size ();
    ROW ("Number of modules", to_string (nmodules).c_str());

    uint32_t ndevices = 0;
    for (ClockDomain &cdom : cdomains)
        ndevices += cdom.modules.size ();
    ROW ("Number of devices", to_string (ndevices).c_str());

    for (auto i = 0; i < cdomains.size(); i++)
    {
        ClockDomain &cdom = cdomains[i];

        float freq = 1.0f / cdom.period * 1000000;
        const char *fieldname = "";
        if (i == 0) fieldname = "Clock frequency (MHz)";

        ROW (fieldname, ("(" + cdom.name + ") " + to_string (freq)).c_str());
    }

    ROW ("Total SRAM size (KB)", "TODO");
    ROW ("Total FF size (KB)", "TODO");

    STROKE;

#undef STROKE
#undef ROW
}

void Simulator::ReportSimulationSummary ()
{
    macrotask ("< Simulation summary >");

#define STROKE PRINT ("%s", string(46, '-').c_str())
#define ROW(f, v) PRINT (" %-29s %14s ", f, v)
        
    double simtime = (double)curtime / 10E9;

    STROKE;
    ROW ("Result", "Value");
    STROKE;

    ROW ("Execution time (s)", to_string(runtime).c_str());
    ROW ("Total SRAM read access", "TODO");
    ROW ("Total SRAM write access", "TODO");

    Component::CycleClass<double> cclass = 
        tb->GetTopComponent(KEY(Simulator))->GetAggregateCycleClass();
    uint32_t nmodules = tb->GetTopComponent(KEY(Simulator))->GetNumChildModules ();
    ROW ("Overall activity (%)", 
            to_string(cclass.active / (cclass.active + cclass.idle) /
                nmodules * 100).c_str());

    for (auto i = 0; i < cdomains.size(); i++)
    {
        ClockDomain &cdom = cdomains[i];

        const char *fieldname = "";
        if (i == 0) fieldname = "Number of cycles";

        ROW (fieldname, ("(" + cdom.name + ") " + to_string (cdom.ncycles)).c_str());
    }
    
    for (auto i = 0; i < pswitches.size(); i++)
    {
        double traffic = (double)pswitches[i]->GetAccumTrafficBytes() / (1 << 20) / simtime;
        const char *fieldname = "";
        if (i == 0) fieldname = "PCIe switch traffic (MB/s)";

        ROW (fieldname, ("(" + pswitches[i]->GetInstanceName() + ") " + 
                    to_string (traffic)).c_str());
    }

    STROKE;

    ROW ("Simulation time (s)", to_string(simtime).c_str());

    double energy = tb->GetTopComponent(KEY(Simulator))->GetAggregateConsumedEnergy ();
    ROW ("Estimated energy (J)", (energy == -1 ? "Unknown" : to_string(energy).c_str()));

    double power = energy / (curtime * 10E-9);
    ROW ("Estimated power (W)", (energy == -1 ? "Unknown" : to_string(power).c_str()));

    STROKE;

#undef STROKE
#undef ROW
}

void Simulator::ReportActivityEvents ()
{
    macrotask ("< Activity and Events >");

#define STROKE PRINT ("%s", string(120, '-').c_str())
#define LABEL(f, v, en, p, e) PRINT (" %-50s  %18s %12s %12s  %s ", f, v, en, p, e)

    STROKE;
    LABEL ("Component Name", "Activity (%)", "Energy (mJ)", "Power (mW)", "Events");
    STROKE;
    ReportComponentRec (tb->GetTopComponent(KEY(Simulator)), 0);
    STROKE;

#undef STROKE
#undef LABEL
}


void Simulator::ReportClockFunctionSchedule (ClockDomain &cdom)
{
    macrotask ("< Scheduled Clock Functions (%s) >", cdom.name.c_str());

#define STROKE PRINT ("%s", string(58, '-').c_str())
#define LABEL(n, v) PRINT (" %-3s   %-50s ", n, v)
#define ROW(n, v) PRINT (" % 3d   %-50s ", n, v)

    STROKE;
    LABEL ("No.", "Clock Function");
    STROKE;
    for (auto i = 0; i < cdom.clockers.size(); i++)
        ROW (i + 1, cdom.clockers[i].GetTagString().c_str());
    STROKE;

#undef STROKE
#undef LABEL
#undef ROW

}


void Simulator::ReportComponentRec (Component *comp, uint32_t level)
{
#define ROW(f, v, en, p, e) PRINT (" %-60s % 9.2lf %12s %12s  %s ", f, v, en, p, e)

    if (Unit *unit = dynamic_cast<Unit *>(comp))
    {
        const Component::CycleClass<uint64_t>& cclass = unit->GetCycleClass ();
        const Component::EventCount<uint64_t>& ecount = unit->GetEventCount ();

        string unit_type_str = "???";
        if (dynamic_cast<Module *>(unit))
            unit_type_str = "Module";
        else if (dynamic_cast<Device *>(unit))
            unit_type_str = "Device";

        string indented_name = string(level * 2, ' ') + "[" + unit_type_str + "] " + unit->GetName();
        if (indented_name.size() > 60)
        {
            indented_name[58] = indented_name[59] = '.';
            indented_name.resize (60);
        }

        string eventstr = "";
        if (ecount.stalled != 0)
            eventstr += string("stalled (") +
                to_string(ecount.stalled) + " cycle(s))";

        double avgactive = (double)cclass.active / (cclass.active + cclass.idle) * 100;
        double oenergy = unit->GetConsumedEnergy (); 
        double energy = oenergy * 1000;
        double power = energy / (curtime * 10E-9);

        ROW (indented_name.c_str(), avgactive, 
                (oenergy == -1) ? "Unknown" : to_string(energy).c_str(),
                (oenergy == -1) ? "Unknown" : to_string(power).c_str(),
                eventstr.c_str());
        
        for (auto p = 0; p < unit->GetNumOutPorts (); p++)
        {
            if (ecount.oport_full[p] != 0)
                PRINT ("%s  - %-6s: full (%lu cycle(s))", 
                        string (level + 2, ' ').c_str(),
                        unit->GetOutPortName(p).c_str(), 
                        ecount.oport_full[p]);
        }
    }
    else
    {
        Component::CycleClass<double> aggcclass;
        Component::EventCount<double> aggecount;
        aggcclass = comp->GetAggregateCycleClass ();
        aggecount = comp->GetAggregateEventCount ();

        string indented_name = string(level * 2, ' ') + 
            "[Component] " + comp->GetName();
        if (indented_name.size() > 60)
        {
            indented_name[58] = indented_name[59] = '.';
            indented_name.resize (60);
        }

        string eventstr = "";
        if (aggecount.stalled != 0)
            eventstr += string("stalled (") + 
                to_string(aggecount.stalled / comp->GetNumChildModules ()) + 
                " cycle(s) / module)";

        double avgactive = aggcclass.active /
            (aggcclass.active + aggcclass.idle) * 100;
        double oenergy = comp->GetAggregateConsumedEnergy (); 
        double energy = oenergy * 1000;
        double power = energy / (curtime * 10E-9);

        ROW (indented_name.c_str(), avgactive,
                (oenergy == -1) ? "Unknown" : to_string(energy).c_str(),
                (oenergy == -1) ? "Unknown" : to_string(power).c_str(),
                eventstr.c_str());

        for (auto it = comp->PathwayBegin (); it != comp->PathwayEnd (); it++)
        {
            Pathway *pathway = *it;
            const Pathway::CycleClass<uint64_t>& pcclass = pathway->GetCycleClass ();
            const Pathway::EventCount<uint64_t>& pecount = pathway->GetEventCount ();

            double avgpactive = (double)pcclass.propagating / 
                (pcclass.propagating + pcclass.idle) * 100;

            string indented_name = string(level * 2 + 2, ' ') + 
                "(Path) " + pathway->GetName();
            if (indented_name.size() > 60)
            {
                indented_name[58] = indented_name[59] = '.';
                indented_name.resize (60);
            }

            string eventstr = "";
            if (pecount.msgdrop != 0)
                eventstr += string ("msgdrop (") + 
                    to_string(pecount.msgdrop) + " msg(s))";

            double oenergy = pathway->GetConsumedEnergy (); 
            double energy = oenergy * 1000;
            double power = energy / (curtime * 10E-9);

            ROW (indented_name.c_str(), avgpactive,
                    (oenergy == -1) ? "Unknown" : to_string(energy).c_str(),
                    (oenergy == -1) ? "Unknown" : to_string(power).c_str(),
                    eventstr.c_str());

            for (auto p = 0; p < pathway->GetNumRHS(); p++)
            {
                if (pecount.rhs_blocked[p] != 0)
                    PRINT ("%s  - %-6s: blocked (%lu cycle(s))", 
                            string (level + 2, ' ').c_str(),
                            pathway->GetRHS(p).GetInstanceName().c_str(), 
                            pecount.rhs_blocked[p]);
            }
        }

        for (auto it = comp->ChildBegin (); it != comp->ChildEnd (); it++)
            ReportComponentRec (*it, level + 1);
    }
#undef ROW
}
