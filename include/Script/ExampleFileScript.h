#pragma once

#include <TSim/Script/FileScript.h>
#include <TSim/Utility/Prototype.h>
#include <Script/ExampleInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;
using namespace TSim;


class ExampleFileScript: public FileScript
{
public:
    ExampleFileScript () : FileScript ("ExampleFileScript",
            Prototype<ExampleInstruction>::Get ()) {}

    virtual Instruction* ParseRawString (string rawstr);
};
