
class Instruction;

class Script
{
public:
    Script (string name);

    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    string GetName ();
    virtual Instruction* NextInstruction () = 0;

private:
    string name;
    bool assigned;
};

