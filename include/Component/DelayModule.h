#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class DelayModule: public Component
{
public:
    DelayModule (string iname, Component *parent);
};
