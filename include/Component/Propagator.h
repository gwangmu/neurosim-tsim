#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class Propagator: public Component
{
public:
    Propagator (string iname, Component *parent, int idx);
};
