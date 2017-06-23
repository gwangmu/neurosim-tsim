#pragma once

#include <Base/Metadata.h>
#include <string>

using namespace std;

struct RegisterWord: public Metadata
{
public:
    RegisterWord (const char *clsname): Metadata (clsname, "") {}
};

