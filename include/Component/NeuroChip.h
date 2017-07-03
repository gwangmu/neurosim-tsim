#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class NeuroChip: public Component
{
public:
    NeuroChip (string iname, Component *parent);
};
