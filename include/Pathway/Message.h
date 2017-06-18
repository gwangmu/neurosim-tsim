#pragma once

#include <Base/Metadata.h>

using namespace std;

struct Message: public Metadata
{
public:
    Message (const char *clsname, string name) : Metadata (clsname, name) {}
};
