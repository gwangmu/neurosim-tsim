#pragma once

#include <cstdio>
#include <cstdlib>


#define PROHIBITED do {                                    \
    fprintf (stderr, "prohibited: (system) "           \
            "(%s, line %d)\n", __FILE__, __LINE__);     \
} while (0)


#define DESIGN_FATAL(msg, iname, ...) do {              \
    fprintf (stderr, "fatal: (%s) [%s, line %u] "         \
            msg "\n", iname, __FILE__, __LINE__,        \
            ##__VA_ARGS__);                               \
    abort ();                                           \
} while(0)

#define SIM_FATAL(msg, iname, ...)                      \
    DESIGN_FATAL(msg, iname, ##__VA_ARGS__)


#define SYSTEM_ERROR(msg, ...) do {                        \
    fprintf (stderr, "error: (system) [%s, line %d] "  \
            msg " \n", __FILE__, __LINE__, ##__VA_ARGS__);\
    abort ();                                           \
} while (0)

#define DESIGN_ERROR(msg, iname, ...) {                 \
    fprintf (stderr, "error: (%s) " msg "\n",            \
            iname, ##__VA_ARGS__);                        \
} while (0)

#define SIM_ERROR(msg, iname, ...)                      \
    DESIGN_ERROR(msg, iname, ##__VA_ARGS__)


#define SYSTEM_WARNING(msg, ...) do {                      \
    fprintf (stderr, "warning: (system) [%s, line %d] " \
            msg "\n",  __FILE__, __LINE__, ##__VA_ARGS__);\
} while (0)

#define DESIGN_WARNING(msg, iname, ...) do {               \
    fprintf (stderr, "warning: (%s) " msg "\n",          \
            iname, ##__VA_ARGS__);                        \
} while (0)

#define SIM_WARNING(msg, iname, ...)                    \
    DESIGN_ERROR(msg, iname, ##__VA_ARGS__)


#define PRINT(msg, ...) do {                               \
    fprintf (stdout, msg "\n", ##__VA_ARGS__);           \
} while (0)


#if defined(MICRODEBUG) || !defined(NDEBUG)
    #define operation(msg, ...) {                           \
        fprintf (stdout, "debug: [%s, line %d] "             \
                msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    }
    #define MICRODEBUG_PRINT(msg, ...)                      \
        operation(msg, ##__VA_ARGS__)
#else
    #define operation(msg, ...)
    #define MICRODEBUG_PRINT(msg, ...)
#endif
    
#ifndef NDEBUG
    #define task(msg, ...) {                                \
        fprintf (stdout, "debug: [%s, line %d] "             \
                msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    }
    #define DEBUG_PRINT(msg, ...) task(msg, ##__VA_ARGS__)
#else
    #define task(msg, ...)
    #define DEBUG_PRINT(msg, ...)
#endif

#define macrotask(msg, ...) {                               \
        fprintf (stdout, msg "\n", ##__VA_ARGS__);            \
    }
