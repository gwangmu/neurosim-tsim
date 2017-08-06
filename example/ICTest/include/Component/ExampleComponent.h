#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace TSim;


class ExampleComponent: public Component
{
public:
    ExampleComponent (string iname, Component *parent);
};
