#pragma once

// NOTE: I know this is not an INTERFACE
class INullable
{
public:
    INullable () { null = false; }

    void Nullify () { null = true; }
    bool IsNull () { return null; }

private:
    bool null;
};
