#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class NeuroCore: public Component
{
public:
    NeuroCore (string iname, Component *parent, int num_propagators);
};
