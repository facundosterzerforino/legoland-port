#ifndef PORT_TRACE_H
#define PORT_TRACE_H

#include <stdarg.h>

/* [port] Debug trace to a text file next to the exe (legoland-port-trace.txt). The original game
 * compiled its DebugTrace calls out; the port routes them here, plus its own PortTrace lines. */
void PortTrace(const char *fmt, ...);
void PortTraceV(const char *fmt, va_list ap);

#endif
