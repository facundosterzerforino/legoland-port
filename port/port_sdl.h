#ifndef PORT_SDL_H
#define PORT_SDL_H

/* [port] SDL3, the platform layer of phase 5 (ROADMAP.md): built from a pinned release as a static library
 * (CMakeLists.txt). The Windows APIs the game calls move onto it one subsystem at a time. */

/* Writes (truncating) the SDL3 version linked into the exe, and the video drivers compiled into it, to path.
 * Returns the number of problems: the library differs from the headers the port was built against. Part of
 * -port-selftest; nothing is initialized and no window opens. */
int PortSdlSelfTest(const char *path);

#endif
