#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class Propagator: public Component
{
public:
    Propagator (string iname, Component *parent, int board_idx, int prop_idx);
};
