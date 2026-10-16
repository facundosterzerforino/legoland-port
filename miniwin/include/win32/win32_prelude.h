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
#include <setjmp.h>

/* Windows' long: 32 bits. The target's long where that's 32 bits (wasm32), else int (64-bit Linux, Android) -- as
 * the compiler's own Windows builtins (_InterlockedIncrement...) declare it. */
#if __SIZEOF_LONG__ == 4
#define __LONG32 long
#else
#define __LONG32 int
#endif

/* the code's wide strings are Windows' 16-bit ones (-fshort-wchar); the C library's wide functions are 32-bit, so
 * the ones the code uses are here */
static inline unsigned short *MiniwinWcscpy(unsigned short *to, const unsigned short *from) {
    unsigned short *start = to;
    while ((*to++ = *from++) != 0) {
    }
    return start;
}
#define wcscpy(to, from) MiniwinWcscpy((unsigned short *)(to), (const unsigned short *)(from))
static inline unsigned short *MiniwinWcscat(unsigned short *to, const unsigned short *from) {
    unsigned short *end = to;
    while (*end != 0) {
        end++;
    }
    MiniwinWcscpy(end, from);
    return to;
}
#define wcscat(to, from) MiniwinWcscat((unsigned short *)(to), (const unsigned short *)(from))

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

/* Structured exception handling off Windows (miniwin/src/posix/seh.c): a __try block runs in a sigsetjmp'd frame;
 * a fault in it (a signal) comes back there with Windows' description of it, and the __except filter decides. A
 * __try body must not leave with return, break or goto, and as with setjmp, locals it changes that are read after a
 * fault must be volatile (the code's __try blocks need neither). */
int PosixTryPush(void);
void *PosixTryTop(void);
void PosixTryPop(void);
int PosixTryFilter(int verdict);
void *PosixExceptionInformation(void);
unsigned int PosixExceptionCode(void);
#define __try \
    if (PosixTryPush() && sigsetjmp(*(sigjmp_buf *)PosixTryTop(), 1) == 0) \
        for (int posix_try_once_ = 1; posix_try_once_; posix_try_once_ = 0, PosixTryPop())
#define __except(...) else if (PosixTryPop(), PosixTryFilter((__VA_ARGS__)))
#define GetExceptionInformation() PosixExceptionInformation()
#define GetExceptionCode() PosixExceptionCode()

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
