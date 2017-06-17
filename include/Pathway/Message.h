
struct Message
{
public:
    Message* GetPrototype ();

protected:
    virtual Message* CreatePrototype () = 0;

private:
    Message *proto;
};
