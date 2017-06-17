#pragma once

#define PERMIT(x) x##AccessKey
#define KEY(x) (x##AccessKey ()) 

#define CREATE_KEY(x)               \
    class x;                        \
    class x##AccessKey              \
    {                               \
        friend class x;             \
        x##AccessKey ();            \
    }

CREATE_KEY(Simulator);
CREATE_KEY(Pathway);
CREATE_KEY(Unit);
