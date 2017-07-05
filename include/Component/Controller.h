#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


class Controller: public Component
{
public:
    Controller (string iname, Component *parent, uint32_t num_board);
};
