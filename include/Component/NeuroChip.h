#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class NeuroChip: public Component
{
public:
    NeuroChip (string iname, Component *parent, 
            int num_cores, int num_propagators,
            uint16_t chip_idx);
};
