/* [port] The resource checks of -port-selftest (port/port_res.c). */
#ifndef PORT_RES_H
#define PORT_RES_H

/* Calls fn with each file name of the resource directory dir (as RES_OpenFile names it, e.g. "Objdesc\\").
   Returns the number of files, or -1 if no open volume has that directory. (src/legoland/resource.c) */
int PortResForEachFile(const char *dir, void (*fn)(const char *name, void *ctx), void *ctx);

/* Opens the resource volumes and reads every object class's fixed part as the game does (PortFromX86), writing
   a dump of the fields to port-selftest-odf.txt (the same text on every build) and a summary line to path.
   Returns the number of problems. */
int PortResSelfTest(const char *path);

#endif
