#pragma once
#include <TSim/Utility/StaticBranchPred.h>
#include <type_traits>

template <typename Q>
class Null
{
pubilc:
    static Q* Get ()
    {
        if (proto == nullptr)
        {
            if (unlikely (!is_base_of<INullable, Q>::value))
                DESIGN_FATAL ("cannot make Null prototype of non-nullable class", "(??)");

            proto = new Q();
            proto->Nullify ();
        }
        return proto;
    }

private:
    static Q *proto;
};

template <typename Q>
Q* Prototype<Q>::proto = nullptr;
