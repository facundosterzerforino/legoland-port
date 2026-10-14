/* [port] Saved games in the original's 32-bit layout on every build. The game writes and reads its structs whole
   (SaveGameWrite(node, sizeof(struct SafariNode))); src/legoland/llidb.h turns those calls into the functions
   below, with the descriptor of the buffer's type (port_x86.h). A 64-bit build converts each struct between its
   own layout and the original's, so its saves are the original's and the original's saves load in it; a 32-bit
   build has the original layout already and reads and writes the bytes as they are. */
#ifndef PORT_SAVE_H
#define PORT_SAVE_H

#include "port_x86.h"

/* count bytes at buffer, a whole number of the type's structs when type isn't NULL */
unsigned int PortSaveRead(void *buffer, unsigned int count, const struct PortX86Type *type);
unsigned int PortSaveWrite(const void *buffer, unsigned int count, const struct PortX86Type *type);

/* the save self-test: every type's table converts an original-layout struct to the port's and back unchanged.
   Writes to report, returns the number of problems. */
int PortSaveSelfTest(void *report);

#endif
