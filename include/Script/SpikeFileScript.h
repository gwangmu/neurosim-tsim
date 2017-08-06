#pragma once

#include <TSim/Script/FileScript.h>
#include <TSim/Utility/Prototype.h>
#include <Script/SpikeInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class SpikeFileScript: public FileScript
{
public:
    SpikeFileScript () : FileScript ("SpikeFileScript",
            Prototype<SpikeInstruction>::Get ()) {}

    virtual Instruction* ParseRawString (string rawstr);
};
