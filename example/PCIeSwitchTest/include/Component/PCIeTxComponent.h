#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class PCIeTxComponent: public Component
{
public:
    PCIeTxComponent (string iname, Component *parent, uint32_t seedid);
};
