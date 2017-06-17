
class Register
{
public:
    enum RegisterType { FF, SRAM, /* and more.. */ };

    struct RegisterAttr
    {
        RegisterAttr (uint32_t wordsize, uint32_t nwords)
        {
            this->wordsize = wordsize;
            this->nwords = nwords;
        }

        uint32_t wordsize;
        uint64_t nwords;
    };

public:
    Register (string name, string datatag, RegisterType type, RegisterAttr attr, RegisterWord *wproto);

    string GetName ();
    RegisterWord* GetWordPrototype ();
    void CheckAssigned () { assigned = true; }
    bool IsAssigned () { return assigned; }

    bool LoadDataFromFile (string filename);
    virtual RegisterWord* ParseRawLine (string rawline) = 0;

    RegisterWord* GetWord (uint64_t addr);

private:
    string name;
    string datatag;
    bool assigned;

    RegisterType type;
    RegisterAttr attr;
    RegisterWord *wproto;

    vector<RegisterWord *> words;
};
