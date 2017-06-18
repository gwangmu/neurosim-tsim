#pragma once

#include <stdio.h>


#define PROHIBITED {                                    \
    fprintf (stderr, "(system): prohibited: "           \
            "(%s, line %d)\n", __FILE__, __LINE__);     \
}


#define DESIGN_FATAL(msg, iname, ...) {                 \
    fprintf (stderr, "%s: fatal: (%s, line %u)"         \
            msg "\n", iname, __FILE__, __LINE__,        \
            __VA_ARGS__);                               \
    abort ();                                           \
}


#define SYSTEM_ERROR(msg, ...) {                        \
    fprintf (stderr, "(system): error: (%s, line %d) "  \
            msg " \n", __FILE__, __LINE__, __VA_ARGS__);\
    abort ();                                           \
}

#define DESIGN_ERROR(msg, iname, ...) {                 \
    fprintf (stderr, "%s: error: " msg "\n",            \
            iname, __VA_ARGS__);                        \
}

#define SIM_ERROR(msg, iname, ...)                      \
    DESIGN_ERROR(msg, iname, __VA_ARGS__)


#define SYSTEM_WARNING(msg, ...) {                      \
    fprintf (stderr, "(system): warning: (%s, line %d)" \
            msg "\n",  __FILE__, __LINE__, __VA_ARGS__);\
}

#define DESIGN_WARNING(msg, iname, ...) {               \
    fprintf (stderr, "%s: warning: " msg "\n",          \
            iname, __VA_ARGS__);                        \
}

#define SIM_WARNING(msg, iname, ...)                    \
    DESIGN_ERROR(msg, iname, __VA_ARGS__)


#define PRINT(msg, ...) {                               \
    fprintf (stdout, msg "\n",                          \
            __FILE__, __LINE__, __VA_ARGS__);           \
}


#if defined(MICRODEBUG) || !defined(NDEBUG)
    #define operation(msg, ...) {                           \
        fprintf (stderr, "debug: (%s, line %d)"             \
                msg "\n", __FILE__, __LINE__, __VA_ARGS__); \
    }
    #define MICRODEBUG_PRINT(msg, ...)                      \
        operation(msg, __VA_ARGS__)
#else
    #define operation(msg, ...)
    #define MICRODEBUG_PRINT(msg, ...)
#endif
    
#ifndef NDEBUG
    #define task(msg, ...) {                                \
        fprintf (stderr, "debug: (%s, line %d)"             \
                msg "\n", __FILE__, __LINE__, __VA_ARGS__); \
    }
    #define DEBUG_PRINT(msg, ...) task(msg, __VA_ARGS__)
#else
    #define task(msg, ...)
    #define DEBUG_PRINT(msg, ...)
#endif

#define macrotask(msg, ...) {                               \
        fprintf (stdout, msg "\n", __VA_ARGS__);            \
    }
