#include <TSim/Simulation/Simulator.h>

#include <TSim/Simulation/Testbench.h>
#include <TSim/Module/Module.h>
#include <TSim/Device/Device.h>
#include <TSim/Register/FileRegister.h>
#include <TSim/Script/FileScript.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <chrono>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;


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

    vector<Module *> modules;
    vector<Device *> devices;
    vector<Pathway *> pathways;
    vector<Pathway *> pathways_postdev;
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
                
                if (!pathway->IsPostDevicePathway ())
                    pathways.push_back (pathway);
                else
                    pathways_postdev.push_back (pathway);
            }

            for (auto icomp = nextcomp->ChildBegin ();
                    icomp != nextcomp->ChildEnd (); icomp++)
            {
                Component *comp = *icomp;
                if (Module *module = dynamic_cast<Module *>(comp))
                    modules.push_back (module);
                else if (Device *device = dynamic_cast<Device *>(comp))
                    devices.push_back (device);
                else
                    queComps.push (comp);
            }
        }

        PRINT ("total %zu module(s), %zu device(s) found", modules.size(), devices.size());
        for (Module *module:modules)
            DEBUG_PRINT ("%s", module->GetName().c_str());
    }

    task ("load simulation spec")
    {
        if (!tb->LoadSimulationSpec (specfilename, KEY(Simulator)))
            DESIGN_FATAL ("cannot load simulation spec '%s'",
                    tb->GetName().c_str(), specfilename.c_str());
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
                (tb->GetUIntParam (Testbench::MODULE_DYNAMIC_POWER, 
                                   module->GetInstanceName(), KEY(Simulator)),
                 KEY(Simulator));
            module->SetStaticPower 
                (tb->GetUIntParam (Testbench::MODULE_STATIC_POWER, 
                                   module->GetInstanceName(), KEY(Simulator)),
                 KEY(Simulator));
        }

        for (Device *device : devices)
        {
            string nclock = device->GetClock ();
            if (nclock == "")
                DESIGN_FATAL ("undefined device clock (device: %s)",
                        tb->GetName().c_str(), device->GetName().c_str());

            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].devices.push_back (device);

            // TODO
            //device->SetDynamicPower 
            //    (tb->GetUIntParam (Testbench::DEVICE_DYNAMIC_POWER, 
            //                       device->GetInstanceName(), KEY(Simulator)),
            //     KEY(Simulator));
            //device->SetStaticPower 
            //    (tb->GetUIntParam (Testbench::DEVICE_STATIC_POWER, 
            //                       device->GetInstanceName(), KEY(Simulator)),
            //     KEY(Simulator));
        }

        for (Pathway *pathway : pathways)
        {
            string nclock = pathway->GetClock ();
            if (nclock == "")
                DESIGN_FATAL ("undefined pathway clock (pathway: %s)",
                        tb->GetName().c_str(), pathway->GetName().c_str());
            
            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].pathways.push_back (pathway);

            pathway->SetDissipationPower 
                (tb->GetUIntParam (Testbench::PATHWAY_DIS_POWER, 
                                   pathway->GetInstanceName(), KEY(Simulator)),
                 KEY(Simulator));
        }

        for (Pathway *pathway : pathways_postdev)
        {
            string nclock = pathway->GetClock ();
            if (nclock == "")
                DESIGN_FATAL ("undefined pathway clock (pathway: %s)",
                        tb->GetName().c_str(), pathway->GetName().c_str());
            
            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].pathways_postdev.push_back (pathway);

            pathway->SetDissipationPower 
                (tb->GetUIntParam (Testbench::PATHWAY_DIS_POWER, 
                                   pathway->GetInstanceName(), KEY(Simulator)),
                 KEY(Simulator));
        }
        
        for (auto &centry : mapCDoms)
            cdomains.push_back (centry.second);

        for (ClockDomain &cdomain : cdomains)
        {
            cdomain.period = tb->GetUIntParam (Testbench::CLOCK_PERIOD, cdomain.name, KEY(Simulator));

            for (Module *module : cdomain.modules)
                module->SetClockPeriod(cdomain.period, KEY(Simulator));

            for (Pathway *pathway : cdomain.pathways)
                pathway->SetClockPeriod (cdomain.period, KEY(Simulator));
        }

        PRINT ("total %zu clock domain(s) formed", cdomains.size());
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

        PRINT ("total %zu file script(s), %zu register(s) found", fscrs.size(), regs.size());

        task ("load file scripts") {
            for (FileScript *fscr : fscrs)
            {
                string pname = fscr->GetParent()->GetInstanceName();
                string path = tb->GetStringParam (Testbench::FILESCRIPT_PATH,
                        pname, KEY(Simulator));

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

        task ("load register data") {
            for (FileRegister *reg : regs)
            {
                string pname = reg->GetParent()->GetInstanceName();
                string path = tb->GetStringParam (Testbench::REGISTER_DATAPATH,
                        pname, KEY(Simulator));

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
                SYSTEM_ERROR ("empty clock domain '%s'", cdom.name.c_str());
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
        uint64_t nexttstime = opt.tsinterval;
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
                PRINT ("Simulating %lu ns..", curtime);
                nexttstime += opt.tsinterval;
            }

            task ("simulate %lu ns", curtime)
            {
                operation ("pre-clock modules");
                for (Module *module : curCDom->modules)
                    module->PreClock (KEY(Simulator));

                operation ("pre-clock pathways");
                for (Pathway *pathway : curCDom->pathways)
                    pathway->PreClock (KEY(Simulator));

                operation ("post-clock modules");
                for (Module *module : curCDom->modules)
                    module->PostClock (KEY(Simulator));

                operation ("devices (preparatory)");
                for (Device *device : curCDom->devices)
                    device->PreClock (KEY(Simulator));

                operation ("post-clock pathways");
                for (Pathway *pathway : curCDom->pathways)
                    pathway->PostClock (KEY(Simulator));

                operation ("post-clock devices");
                for (Device *device : curCDom->devices)
                    device->PostClock (KEY(Simulator));

                operation ("post-clock pathways (post-devices)");
                for (Pathway *pathway : curCDom->pathways_postdev)
                    pathway->PostClock (KEY(Simulator));
            }

            if (curtime > opt.timelimit) 
            {
                PRINT ("Simulation reached time limit (%lu ns)", opt.timelimit);
                break;
            }
        }
    }

    auto end = chrono::steady_clock::now ();
    runtime = chrono::duration_cast<chrono::milliseconds>(end - start).count ();

    PRINT ("Simulation finished at %lu ns", curtime);

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
#define ROW(f, v) PRINT (" %-29s %14s ", f, v);

    STROKE;
    ROW ("Specification", "Value");
    STROKE;

    uint32_t nmodules = 0;
    for (ClockDomain &cdom : cdomains)
        nmodules += cdom.modules.size ();
    ROW ("Number of modules", to_string (nmodules).c_str());

    for (auto i = 0; i < cdomains.size(); i++)
    {
        ClockDomain &cdom = cdomains[i];

        float freq = 1.0f / cdom.period * 1000;
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
#define ROW(f, v) PRINT (" %-29s %14s ", f, v);
        
    STROKE;
    ROW ("Result", "Value");
    STROKE;

    ROW ("Execution time (s)", to_string((double)runtime / 10E3).c_str());
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

    STROKE;

    ROW ("Simulation time (s)", to_string((double)curtime / 10E9).c_str());

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
#define LABEL(f, v, en, p, e) PRINT (" %-50s  %18s %12s %12s  %s ", f, v, en, p, e);

    STROKE;
    LABEL ("Component Name", "Activity (%)", "Energy (mJ)", "Power (mW)", "Events");
    STROKE;
    ReportComponentRec (tb->GetTopComponent(KEY(Simulator)), 0);
    STROKE;

#undef STROKE
#undef ROW
}


void Simulator::ReportComponentRec (Component *comp, uint32_t level)
{
#define ROW(f, v, en, p, e) PRINT (" %-60s % 9.2lf %12s %12s  %s ", f, v, en, p, e);

    if (Module *module = dynamic_cast<Module *>(comp))
    {
        const Component::CycleClass<uint64_t>& cclass = module->GetCycleClass ();
        const Component::EventCount<uint64_t>& ecount = module->GetEventCount ();

        string indented_name = string(level * 2, ' ') + "[Module] " + module->GetName();
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
        double oenergy = module->GetConsumedEnergy (); 
        double energy = oenergy * 1000;
        double power = energy / (curtime * 10E-9);

        ROW (indented_name.c_str(), avgactive, 
                (oenergy == -1) ? "Unknown" : to_string(energy).c_str(),
                (oenergy == -1) ? "Unknown" : to_string(power).c_str(),
                eventstr.c_str());
        
        for (auto p = 0; p < module->GetNumOutPorts (); p++)
        {
            if (ecount.oport_full[p] != 0)
                PRINT ("%s  - %-6s: full (%lu cycle(s))", 
                        string (level + 2, ' ').c_str(),
                        module->GetOutPortName(p).c_str(), 
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
