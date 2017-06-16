
struct RegisterWord {};

class Register
{
    enum RegisterType { FF, SRAM, /* and more.. */ }

public:
    Register (string tag, RegisterType type, uint32_t wordsize, uint64_t nwords);

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawLine (string rawline) = 0;

    RegisterWord* GetWord (uint64_t addr);

private:
    string tag;
    RegisterType type;
    uint32_t wordsize;
    uint64_t nwords;

    vector<RegisterWord *> words;
};
