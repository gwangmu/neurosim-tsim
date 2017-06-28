#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Interface/INullable.h>
#include <string>

using namespace std;

struct RegisterWord: public Metadata, public INullable
{
public:
    RegisterWord (const char *clsname): Metadata (clsname, "") {}
};

