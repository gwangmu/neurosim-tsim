#pragma once

#include <TSim/Register/RegisterWord.h>
#include <string>

using namespace std;

struct DelayRegisterWord: public RegisterWord
{
public:
    // NOTE: must provide default constructor
    DelayRegisterWord () 
        : RegisterWord ("DelayRegisterWord"), next_addr (0),
            addr_sub(0), val16(0) {}

    DelayRegisterWord (uint32_t next_addr, uint64_t addr_sub, uint16_t val16)
        : RegisterWord ("DelayRegisterWord"), next_addr (next_addr),
            addr_sub (addr_sub), val16 (val16) {}
    
    uint32_t next_addr;
    uint64_t addr_sub;
    uint16_t val16;
};
