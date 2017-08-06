#pragma once

#include <TSim/Base/Metadata.h>
#include <TSim/Utility/AccessKey.h>
#include <string>

using namespace std;

namespace TSim
{
    struct RegisterWord;
    class Module;

    class Register: public Metadata
    {
    public:
        enum Type { FF, SRAM, DRAM };
    
        struct Attr
        {
            Attr (uint32_t wordsize, uint32_t addrsize)
            {
                this->wordsize = wordsize;
                this->addrsize = addrsize;
            }
    
            uint32_t wordsize;
            uint64_t addrsize;
        };
    
    public:
        Register (const char *clsname, Type type, Attr attr, RegisterWord *wproto);
    
        inline const Type& GetType () { return type; }
        inline const Attr& GetAttr () { return attr; }
        inline RegisterWord* GetWordPrototype () { return wproto; }
    
        Module* GetParent () { return parent; }
        void SetParent (Module *module, PERMIT(Module)) { parent = module; }
        uint64_t GetByteCapacity () { return attr.addrsize * attr.wordsize / 8; }
        uint32_t GetReadCount () { return rdcount; }
        uint32_t GetWriteCount () { return wrcount; }
    
        /* Called by 'Simulator' */
        void SetReadEnergy (double rdenergy) { this->rdenergy = rdenergy; }
        void SetWriteEnergy (double wrenergy) { this->wrenergy = wrenergy; }
        void SetStaticPower (uint32_t stapower) { this->stapower = stapower; }
        double GetReadEnergy () { return rdenergy; }
        double GetWriteEnergy () { return wrenergy; }
        uint32_t GetStaticPower () { return stapower; }
    
        double GetConsumedStaticEnergy ();
        double GetAccumReadEnergy ();
        double GetAccumWriteEnergy ();
    
        // TODO: need to be optimized
        virtual const RegisterWord* GetWord (uint64_t addr) = 0;
        virtual bool SetWord (uint64_t addr, RegisterWord *word) = 0;
    
        /* Called by derived 'Register' */
        void IncrReadCount () { rdcount++; }
        void IncrWriteCount () { wrcount++; }
    
    protected:
        virtual bool InitWord (uint64_t addr, RegisterWord *word) = 0;
    
    private:
        Module *parent;
    
        Type type;
        Attr attr;
        RegisterWord *wproto;
    
        double rdenergy;
        double wrenergy;
        uint32_t stapower;
        uint32_t rdcount;
        uint32_t wrcount;
    };
}

