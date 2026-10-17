#ifndef PORT_GLOBALS_LAYOUT_H
#define PORT_GLOBALS_LAYOUT_H

#include <stddef.h>

/* the game's globals in the original's address order (port_globals_layout.c, generated) */
struct PortLayoutGlobal {
    unsigned int original; /* its address in legoland.exe */
    const char *name;
    const void *address; /* here */
    size_t size; /* here */
};

extern const struct PortLayoutGlobal PortLayoutGlobals[];
extern const int PortLayoutGlobalCount;

#endif
