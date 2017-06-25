#include <TSim/Simulation/Simulator.h>

#include <TSim/Simulation/Testbench.h>
#include <TSim/Module/Module.h>
#include <TSim/Register/Register.h>
#include <TSim/Script/FileScript.h>
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


Simulator::Simulator ()
{
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
        queComps.push (tb->LoadTopComponent (KEY(Simulator)));
        
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
    }

    task ("init this->cdomains")
    {
        map<string, ClockDomain> mapCDoms;

        for (Module *module : modules)
        {
            string nclock = module->GetClock ();

            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].modules.push_back (module);
        }

        for (Pathway *pathway : pathways)
        {
            // NOTE: pathway follows clock domain of LHS module.
            // NOTE: all LHS endpoints must be in the same clock domain.
            string nclock = pathway->GetEndpoint(Endpoint::LHS)->
                GetConnectedModule()->GetClock ();
            
            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].pathways.push_back (pathway);
        }
        
        for (auto &centry : mapCDoms)
            cdomains.push_back (centry.second);

        PRINT ("total %zu clock domain(s) formed", cdomains.size());
    }

    task ("init this->fscrs, this->regs")
    {
        for (Module *module : modules)
        {
            if (FileScript *fscr = dynamic_cast<FileScript *>(module->GetScript ()))
                fscrs.push_back (fscr);

            if (Register *reg = module->GetRegister ())
                regs.push_back (reg);
        }

        task ("load file scripts") {
            for (FileScript *fscr : fscrs)
            {
                string fscrname = fscr->GetName ();
                string path = tb->GetStringParam (Testbench::FILESCRIPT_PATH,
                        fscrname, KEY(Simulator));

                if (path != "")
                {
                    PRINT ("Loading '%s' <-- '%s'..", 
                            fscr->GetName().c_str(), path.c_str());
                    fscr->LoadScriptFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s' specified",
                            tb->GetName().c_str(),
                            fscr->GetName().c_str());
            }
        }

        task ("load register data") {
            for (Register *reg : regs)
            {
                string regname = reg->GetName ();
                string path = tb->GetStringParam (Testbench::REGISTER_DATAPATH,
                        regname, KEY(Simulator));

                if (path != "")
                {
                    PRINT ("Loading '%s' <-- '%s'..", 
                            reg->GetName().c_str(), path.c_str());
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
        IssueCount icount = tb->LoadTopComponent(KEY(Simulator))->
            Validate(KEY(Simulator));
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
        }
    }

    return true;
}
