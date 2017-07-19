#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class FastCore: public Component
{
public:
    FastCore (string iname, Component *parent, int num_propagators);
};
