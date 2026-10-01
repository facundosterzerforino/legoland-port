#pragma once

/* [port] The original program's initialized data (generated into port_data.c by tools/gen_data.py). */

/* Gives the port's globals their original starting values. Call once, first thing at startup. */
void PortLoadData(void);

/* Checks what PortLoadData did and writes a report to path. Returns the number of problems found. */
int PortDataSelfTest(const char *path);

/* The original initialized byte at virtual address va (0x4ab000..0x4c2000, already pointer-patched), or NULL. */
const void *PortOriginalData(unsigned int va);
