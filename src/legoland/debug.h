#pragma once

#include <windows.h>
#include "legoland.h"

#ifdef LEGOLAND_PORT
/* [port] The game's function only borrowed the name wWinMain; modern SDKs declare wWinMain as the Unicode
   program entry (WINAPI, LPWSTR), which conflicts. Rename it in the port build (after <windows.h>, so the
   SDK's own declaration keeps its name). */
#define wWinMain LegolandMain
#endif

int __cdecl wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void FUN_0047f870(const char *fmt, ...);
int FUN_0047f880(void);
LEGO_EXPORT unsigned int mystrlen(const char *s);
