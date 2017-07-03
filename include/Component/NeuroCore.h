#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class NeuroCore: public Component
{
public:
    NeuroCore (string iname, Component *parent, int num_propagators);
};
