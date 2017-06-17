#include <Simulation/Simulator.h>
#include <Simulation/Testbench.h>


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
    macrotask ("Loading '%s %s'..", tb->GetClassName(), tb->GetName());

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
                    ipath != nextcomp->PathwayEnd (KEY(Simulator));
                    ipath = nextcomp->PathwayNext (ipath, KEY(Simulator)))
            {
                pathways.push_back (*ipath);
            }

            for (auto iface = nextcomp->ChildBegin (KEY(Simulator));
                    iface != nextcomp->ChildEnd (KEY(Simulator));
                    iface = nextcomp->ChildNext (iface, KEY(Simulator)))
            {
                Interface *face = *iface;
                if (Component *comp = dynamic_cast<Component *>(face))
                    queComps.enqueue (comp);
                else if (Module *module = dynamic_cast<Module *>(face))
                    modules.push_back (module);
                else
                    SYSTEM_ERROR ("bogus Interface class");
            }
        }

        PRINT ("total %d module(s) found", modules.size());
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
            string nclock = pathway->GetLHSEndpt()->
                GetConnectedModule()->GetClock ();
            
            mapCDoms[nclock].name = nclock;
            mapCDoms[nclock].pathways.push_back (pathway);
        }
        
        for (auto &centry : mapCDoms)
            cdomains.push_back (centry.second);

        PRINT ("total %d clock domain(s) formed", cdomains.size());
    }

    task ("init this->fscrs, this->regs")
    {
        for (Module *module : modules)
        {
            if (FileScript *fscr = dynamic_cast<FileScript *>(module->GetScript ()))
                fscrs->push_back (fscr);

            if (Register *reg = module->GetRegister ())
                regs->push_back (reg);
        }

        task ("load file scripts") {
            for (FileScript *fscr : fscrs)
            {
                string fscrname = fscr->GetName ();
                string path = tb->GetStringParam (Testbench::FILESCRIPT_PATH,
                        fscrname, KEY(Simulator));

                if (path != "")
                {
                    PRINT ("Loading '%s %s' <-- '%s'..", 
                            fscr->GetClassName().c_str(),
                            fscr->GetFullName().c_str(), path.c_str())
                    fscr->LoadScriptFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s %s' specified",
                            tb->GetClassName().c_str(),
                            fscr->GetClassName().c_str(),
                            fscr->GetFullName().c_str());
            }
        }

        task ("load register data") {
            for (Register *reg : regs)
            {
                string regname = reg->GetName ();
                string path = tb->GetStringParam (Testbench::REGISTER_PATH,
                        regname, KEY(Simulator));

                if (path != "")
                {
                    PRINT ("Loading '%s %s' <-- '%s'..", 
                            reg->GetClassName().c_str(),
                            reg->GetFullName().c_str(), path.c_str())
                    reg->LoadDataFromFile (path);
                }
                else
                    DESIGN_WARNING ("no file for '%s %s' specified",
                            tb->GetClassName().c_str(), 
                            reg->GetClassName().c_str(),
                            reg->GetFullName()).c_str();
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
        IssueCount icount = tb->GetTopComponent(KEY(Simulator))->Validate ();
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
            DESIGN_ERROR ("no clock found", tb->GetClassName().c_str());
            nerr++;
        }

        for (ClockDomain &cdom : cdomains)
        {
            if (cdom.period == 0)
            {
                DESIGN_ERROR ("clock '%s' has 0 period",
                        tb->GetClassName().c_str(), cdom->name.c_str());
                nerr++;
            }

            if (cdom.modules.empty())
                SYSTEM_ERROR ("empty clock domain '%s'", cdom->name.c_str());
        }

        PRINT ("%d clock error(s)", nerr);
        
        if (nerr)
            return false;
    }

    return true;
}


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
        while (!tb->IsFinished ())
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

            task ("simulate %u ns", curtime)
            {
                operation ("operate modules")
                {
                    for (Module *module : curCDom->modules)
                        module->Clock (KEY(Simulator));
                }

                operation ("flow messages")
                {
                    for (Pathway *pathway : curCDom->pathway)
                        pathway->Clock (KEY(Simulator));
                }
            }
        }
    }

    return true;
}
