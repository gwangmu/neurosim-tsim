#pragma once

class IClockable
{
public:
    virtual void PreClock (PERMIT(Simulator)) = 0;
    virtual void PostClock (PERMIT(Simulator)) = 0;
};
