
class Interface
{
public: 
    /* Universal */
    Interface (string name);
    string GetName ();

protected:  
    /* Called by parent 'Component' */
    bool SetClock (string clockname);
    virtual bool Connect (uint32_t portid, Endpoint *endpt) = 0;

private:
    string name;
    string clockname;
};
