#pragma once

template <typename T>
class Prototype
{
public:
    static T* Get ()
    {
        if (proto == nullptr)
            proto = new T();
        return proto;
    }

private:
    static T *proto;
};

template <typename T>
T* Prototype<T>::proto = nullptr;
