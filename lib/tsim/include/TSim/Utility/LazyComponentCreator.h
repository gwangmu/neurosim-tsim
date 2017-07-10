#pragma once

class Component;

class LazyComponentCreatorBase
{
public:
    virtual Component* Create () = 0;
};

template <typename COMP_TYPE>
class LazyComponentCreator: public LazyComponentCreatorBase
{
public:
    virtual Component* Create () 
    { 
        return new COMP_TYPE ("top", nullptr); 
    }
};
