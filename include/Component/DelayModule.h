#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class DelayModule: public Component
{
public:
    DelayModule (string iname, Component *parent);
};
