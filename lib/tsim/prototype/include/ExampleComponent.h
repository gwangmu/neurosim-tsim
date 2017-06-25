/***
 * TSim Component prototype (header)
 **/

#pragma once

#include <TSim/Base/Component.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <map>

using namespace std;


// (TODO) replace 'ExampleComponent' to the name you want to put
class ExampleComponent: public Component
{
public:
    ExampleComponent (string iname, Component *parent);

private:
    // (OPTIONAL) insert methods/fields you want to add
};
