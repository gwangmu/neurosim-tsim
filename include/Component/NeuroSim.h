#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class NeuroSim: public Component
{
public:
    NeuroSim (string iname, Component *parent, int board_idx);
};
