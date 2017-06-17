
struct Message: public IMetadata
{
public:
    Message (const char *clsname) { this->clsname = clsname; }

    virtual string GetName final { return "(noname)"; }
    virtual string GetFullName final { return "(noname)"; }
    virtual const char* GetClassName final { return clsname; }
    virtual string GetSummary {} ;

private:
    const char *clsname;
};
