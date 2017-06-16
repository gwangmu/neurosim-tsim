
struct ScriptLine {};

class Script
{
public:
    Script (string tag);

    bool LoadScriptFromFile (string filename);
    virtual ScriptLine* ParseRawLine (string rawline) = 0;

    ScriptLine* GetLine ();

private:
    string tag;
    vector<ScriptLine *> scrlines;
};
