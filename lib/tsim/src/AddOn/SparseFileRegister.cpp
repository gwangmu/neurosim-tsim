#include <TSim/AddOn/SparseFileRegister.h>
#include <TSim/AddOn/Element/RegisterWord.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <map>

using namespace std;

struct RegisterWord;
class Simulator;


SparseFileRegister::SparseFileRegister (const char *clsname, string iname, 
        Type type, Attr attr, RegisterWord *wproto)
    : FileRegister (clsname, iname, type, attr, wproto) {}

const RegisterWord* SparseFileRegister::GetWord (uint64_t addr)
{
    if (unlikely (addr > GetAttr().addrsize))
        SIM_FATAL ("accessing out-of-bound address (addr: %lu, parent: %s)", 
                "SparseFileRegister", addr, GetParent()->GetName().c_str());
    
    if (!words.count (addr))
        return nullptr;
    else
        return words[addr];
}

bool SparseFileRegister::SetWord (uint64_t addr, RegisterWord *word)
{
    if (unlikely (addr > GetAttr().addrsize))
    {
        SIM_FATAL ("writing out-of-bound address (parent: %s)", 
                "SparseFileRegister", GetParent()->GetName().c_str());
        return false;
    }

    delete words[addr];
    words[addr] = word;

    return true;
}
