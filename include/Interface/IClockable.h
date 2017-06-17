
class IClockable
{
public:
    virtual void Clock (PERMIT(Simulator)) = 0;
};
