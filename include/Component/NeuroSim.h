#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class NeuroSim: public Component
{
public:
    NeuroSim (string iname, Component *parent, int board_idx);
};
