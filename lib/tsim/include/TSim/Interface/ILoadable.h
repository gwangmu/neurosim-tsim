#pragma once

#include <string>

using namespace std;

class ILoadable
{
public:
    ILoadable () {}

    virtual bool LoadFromFile (string filename) = 0;
    virtual bool IsLoaded () = 0;
};
