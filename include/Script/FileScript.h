
class FileScript: public Script
{
public:
    Script (string tag);

    bool LoadScriptFromFile (string filename);
    virtual Instruction* ParseRawString (string rawstr) = 0;

private:
    string tag;
    vector<Instruction *> instrs;
};
