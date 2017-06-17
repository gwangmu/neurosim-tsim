
class Register
{
    enum RegisterType { FF, SRAM, /* and more.. */ }

public:
    Register (string name, string datatag, RegisterType type, uint32_t wordsize, uint64_t nwords);

    string GetName ();

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawLine (string rawline) = 0;

    RegisterWord* GetWord (uint64_t addr);

private:
    string name;

    string datatag;
    RegisterType type;
    uint32_t wordsize;
    uint64_t nwords;

    vector<RegisterWord *> words;
};
