#pragma once

#include <Script/Script.h>
#include <string>
#include <vector>

using namespace std;

struct Instruction;

class FileScript: public Script
{
public:
    FileScript (string tag);

    bool LoadScriptFromFile (string filename);
    virtual Instruction* ParseRawString (string rawstr) = 0;

private:
    string tag;
    vector<Instruction *> instrs;
};
