
class Pathway;

class Endpoint: public IClockable
{
public:
    Endpoint (Pathway *parent, PERMIT(Pathway));

    // Endpoint IO Operations
    virtual bool Assign (Message msg);
    virtual Message Fetch ();

    // Properties
    Pathway* Parent ();

    // Simulation
    virtual void Clock ();

    // Connection
    bool JoinTo (Port *port, PERMIT(Unit));

private:
    Pathway* parent;
    Port* portConnected;
    Register* reg;
};
