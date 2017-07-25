#include <TSim/Register/DenseFileRegister.h>
#include <TSim/Register/RegisterWord.h>
#include <TSim/Utility/AccessKey.h>
#include <TSim/Utility/StaticBranchPred.h>

#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

struct RegisterWord;
class Simulator;


DenseFileRegister::DenseFileRegister (const char *clsname, Type type, 
        Attr attr, RegisterWord *wproto)
    : FileRegister (clsname, type, attr, wproto)
{
    words.resize (attr.addrsize);
}

const RegisterWord* DenseFileRegister::GetWord (uint64_t addr)
{
    if (unlikely (addr > words.size ()))
        SIM_FATAL ("accessing out-of-bound address (addr: 0x%lu/%lu)", 
                GetName().c_str(), addr, words.size());

    return words[addr];
}

bool DenseFileRegister::SetWord (uint64_t addr, RegisterWord *word)
{
    if (unlikely (addr > words.size ()))
    {
        SIM_FATAL ("writing out-of-bound address (addr: 0x%lu/%lu)", 
                GetName().c_str(), addr, words.size());
        return false;
    }

    delete words[addr];
    words[addr] = word;

    return true;
}
