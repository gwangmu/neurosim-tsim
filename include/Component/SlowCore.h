#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class SlowCore: public Component
{
public:
    SlowCore (string iname, Component *parent, int num_propagators, int idx);
};
