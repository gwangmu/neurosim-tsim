#pragma once

template <typename T>
class Prototype
{
public:
    static void Get ()
    {
        if (proto == nullptr)
            proto = new T();
        return proto;
    }

private:
    static T *proto;
};
