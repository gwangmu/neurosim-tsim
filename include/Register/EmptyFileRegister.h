#pragma once

#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Utility/Prototype.h>
#include <Register/InputRegisterWord.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;
using namespace TSim;


class EmptyFileRegister: public Register
{
public:
    EmptyFileRegister (Type type, Attr attr)
        : Register ("EmptyFileRegister", type, attr, 
                Prototype<InputRegisterWord>::Get()) {}

};
