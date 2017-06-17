
class Interface
{
public: 
    /* Universal */
    Interface ();

    string GetClock ();

protected:  
    /* Called by parent 'Component' */
    bool SetClock (string clockname);
    virtual bool Connect (uint32_t portid, Endpoint *endpt) = 0;

private:
    string clockname;
};
