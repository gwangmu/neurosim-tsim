#pragma once

#include <TSim/Base/Component.h>
#include <Component/TimestepReporter.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;

class NeuroSimSystem: public Component
{
public:
    NeuroSimSystem (string iname, Component *parent);

private:
    TimestepReporter *tsrep;
};
