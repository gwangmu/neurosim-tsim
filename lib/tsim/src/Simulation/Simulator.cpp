#include <TSim/Simulation/Simulator.h>

#include <TSim/Simulation/Testbench.h>
#include <TSim/Module/Module.h>
#include <TSim/AddOn/AddOn.h>
#include <TSim/AddOn/Register.h>
#include <TSim/AddOn/FileScript.h>
#include <TSim/Interface/ILoadable.h>
#include <TSim/Pathway/Pathway.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/Logging.h>

#include <vector>
#include <map>

using namespace std;


/* Constructors */
Simulator::ClockDomain::ClockDomain ()
{
    name = "";
    period = 0;
    nexttime = 0;
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

    vector<Module *> modules;
    vector<Pathway *> pathways;
    task ("find modules/pathways")
    {
        queue<Component *> queComps;
        queComps.push (tb->GetTopComponent (KEY(Simulator)));
        
        while (!queComps.empty ())
        {
            Component *nextcomp = queComps.front ();
            queComps.pop ();

            for (auto ipath = nextcomp->PathwayBegin (KEY(Simulator));
                    ipath != nextcomp->PathwayEnd (KEY(Simulator)); ipath++)
            {
                pathways.push_back (*ipath);
            }

            for (auto icomp = nextcomp->ChildBegin (KEY(Simulator));
                    icomp != nextcomp->ChildEnd (KEY(Simulator)); icomp++)
            {
                Component *comp = *icomp;
                if (Module *module = dynamic_cast<Module *>(comp))
                    modules.push_back (module);
                else
                    queComps.push (comp);
            }
        }

        PRINT ("total %zu module(s) found", modules.size());
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
        }

        for (Pathway *pathway : pathways)
        {
            string nclock = pathway->GetClock ();
            if (nclock == "")
                DESIGN_FATAL ("undefined pathway clock (pathway: %s)",
                        tb->GetName().c_str(), pathway->GetName().c_str());
            
            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].pathways.push_back (pathway);
        }
        
        for (auto &centry : mapCDoms)
            cdomains.push_back (centry.second);

        for (ClockDomain &cdomain : cdomains)
            cdomain.period = tb->GetUIntParam (Testbench::CLOCK_PERIOD, cdomain.name, KEY(Simulator));

        PRINT ("total %zu clock domain(s) formed", cdomains.size());
    }

    task ("init this->fscrs, this->regs")
    {
        for (Module *module : modules)
        {
            if (AddOn *fscr = dynamic_cast<AddOn *>(module->GetScript ()))
            {
                DEBUG_PRINT ("found '%s'", fscr->GetFullName().c_str());
                ld_addons.push_back (fscr);
            }

            if (AddOn *reg = dynamic_cast<AddOn *>(module->GetRegister ()))
            {
                DEBUG_PRINT ("found '%s'", reg->GetFullName().c_str());
                ld_addons.push_back (reg);
            }
        }

        PRINT ("total %zu loadable add-ons found", ld_addons.size());

        task ("load loadable add-ons")
        {
            for (AddOn *ld_addon : ld_addons)
            {
                ILoadable *loadable = dynamic_cast<ILoadable *>(ld_addon);
                if (!loadable)
                    SYSTEM_ERROR ("ld_addon must be able to be castable to ILoadable");

                string pname = ld_addon->GetInstanceName();
                string path = tb->GetStringParam (Testbench::LOADABLE_PATH,
                        pname, KEY(Simulator));

                if (path != "")
                {
                    PRINT ("Loading '%s' <-- '%s'..", 
                            ld_addon->GetFullName().c_str(), path.c_str());
                    loadable->LoadFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s' specified",
                            tb->GetName().c_str(),
                            ld_addon->GetName().c_str());
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

        for (AddOn *ld_addon : ld_addons)
        {
            ILoadable *loadable = dynamic_cast<ILoadable *>(ld_addon);
            if (!loadable)
                SYSTEM_ERROR ("ld_addon must be able to be castable to ILoadable");

            if (!loadable->IsLoaded ())
            {
                SIM_ERROR ("loadable addon '%s' not loaded",
                        tb->GetName().c_str(), ld_addon->GetFullName().c_str());
                icount.error++;
            }
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

                operation ("post-clock pathways");
                for (Pathway *pathway : curCDom->pathways)
                    pathway->PostClock (KEY(Simulator));
            }

            if (curtime > opt.timelimit) 
            {
                PRINT ("Simulation reached time limit (%lu ns)", opt.timelimit);
                break;
            }
        }
    }

    PRINT ("Simulation finished at %lu ns", curtime);

    return true;
}
