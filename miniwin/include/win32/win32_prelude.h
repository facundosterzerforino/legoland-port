/* miniwin/include/win32/win32_prelude.h: what win32.h (generated) takes for granted, for non-Windows builds:
 * calling conventions and SDK annotations as nothing, Windows' integer types by size. */
#ifndef MINIWIN_WIN32_PRELUDE_H
#define MINIWIN_WIN32_PRELUDE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define WINAPI
#define WINAPIV
#define APIENTRY
#define CALLBACK
#define PASCAL
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define CDECL
#define NTAPI
#define FAR
#define NEAR
#define far
#define near
#define CONST const
#define IN
#define OUT
#define OPTIONAL
#define UNALIGNED
#define FORCEINLINE static inline
#define DECLSPEC_IMPORT
#define DECLSPEC_NORETURN
#define DECLSPEC_SELECTANY
#define DECLSPEC_NOTHROW
#define WINBASEAPI
#define WINUSERAPI
#define WINGDIAPI
#define WINMMAPI
#define WINADVAPI
#define WINSPOOLAPI
#define WINCOMMCTRLAPI
#define WINOLEAPI
#define EXTERN_C extern
#define __stdcall
#define __cdecl
#define __fastcall
#define __unaligned
#define __forceinline static inline __attribute__((always_inline))
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#define __declspec(x)

/* Structured exception handling: none off Windows. A __try block just runs; its handler never does (the port's
 * crash report on Linux comes from a signal handler instead). */
#define __try if (1)
#define __except(filter) else if (0)
#define __finally
#define GetExceptionInformation() ((void *)0)
#define GetExceptionCode() 0

/* the compiler's byte swaps, under MSVC's names */
#define _byteswap_ushort __builtin_bswap16
#define _byteswap_ulong __builtin_bswap32
#define _byteswap_uint64 __builtin_bswap64

/* COM, C style: the interface's vtable pointer first */
#define interface struct
#define STDMETHOD(m) HRESULT (*m)
#define STDMETHOD_(t, m) t (*m)
#define PURE
#define THIS_ INTERFACE *This,
#define THIS INTERFACE *This
#define DECLARE_INTERFACE(i) \
    typedef interface i { const struct i##Vtbl *lpVtbl; } i; \
    typedef const struct i##Vtbl i##Vtbl; \
    const struct i##Vtbl
#define DECLARE_INTERFACE_(i, b) DECLARE_INTERFACE(i)
#define BEGIN_INTERFACE
#define END_INTERFACE

#endif
