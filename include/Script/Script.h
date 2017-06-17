
class Instruction;

class Script
{
public:
    Script (string name);

    string GetName ();
    virtual Instruction* NextInstruction () = 0;

private:
    string name;
};

