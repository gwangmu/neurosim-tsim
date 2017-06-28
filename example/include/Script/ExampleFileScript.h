#pragma once

#include <TSim/AddOn/FileScript.h>
#include <TSim/Utility/Prototype.h>
#include <Script/ExampleInstruction.h>

#include <cinttypes>
#include <string>
#include <vector>

using namespace std;


class ExampleFileScript: public FileScript
{
public:
    ExampleFileScript (string iname) 
        : FileScript ("ExampleFileScript", iname,
                Prototype<ExampleInstruction>::Get ()) {}

    virtual Instruction* ParseRawString (string rawstr);
};
