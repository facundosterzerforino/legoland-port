#ifndef PORT_TRACE_H
#define PORT_TRACE_H

#include <stdarg.h>

/* [port] Debug trace to a text file next to the exe (legoland-port-trace.txt). The original game
 * compiled its DebugTrace calls out; the port routes them here, plus its own PortTrace lines. */
void PortTrace(const char *fmt, ...);
void PortTraceV(const char *fmt, va_list ap);

/* [port] Offset of a code address from the exe's load address, to symbolize trace lines with the .pdb
 * (llvm-symbolizer --obj=legoland.exe 0x400000+rva). PORT_CALLER is the current function's return address. */
unsigned long PortRva(const void *address);
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#define PORT_CALLER() _ReturnAddress()
#else
#define PORT_CALLER() __builtin_return_address(0)
#endif

#endif
