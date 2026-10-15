#ifndef LEGOLAND_H
#define LEGOLAND_H

#include <basetsd.h>

#define STUB() ((void)0)

#if defined(_MSC_VER) && !defined(__clang__)
#define LEGO_EXPORT __declspec(dllexport)
#else
#define LEGO_EXPORT
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef union TileId {
    unsigned short id;
    struct {
        unsigned char x;
        unsigned char y;
    } pos;
} TileId;

/* Windows' ACMSTREAMHEADER (msacm.h): its user fields are pointer-sized and its driver area is 15 dwords on 64-bit
 * Windows; the ACM checks cbStruct against that size. */
#ifdef _WIN64
#define ACM_DRV_RESERVED 15
#else
#define ACM_DRV_RESERVED 10
#endif

struct AcmHeader {
    unsigned int cbStruct;
    unsigned int fdwStatus;
    UINT_PTR dwUser;
    unsigned char *pbSrc;
    unsigned int cbSrcLength;
    unsigned int cbSrcLengthUsed;
    UINT_PTR dwSrcUser;
    unsigned char *pbDst;
    unsigned int cbDstLength;
    unsigned int cbDstLengthUsed;
    UINT_PTR dwDstUser;
    unsigned int reserved[ACM_DRV_RESERVED];
};

struct Bloke;

/* Track and element name pair (DAT_004bb0a4); here so globals.h and interface.h both see it. */
struct TrackElemPair {
    /* 0x00 */ char *track_name;
    /* 0x04 */ char *elem_name;
};

/* What the cursor is over (Hover at 0x004bdd00): a type code (0x100 nothing, 0x103 an object, ...),
   the thing itself and its tile.  Passed by value to PopUpInfoSetUp. */
struct HoverInfo {
    int type;
    struct Bloke *ptr; /* or, for an object, its struct Element */
    union {
        unsigned int value;
        TileId tile;
    } data;
};

typedef int (*ScriptCommandFn)(char **args, int nargs, int flags);
struct ScriptCommand {
    const char *name;
    ScriptCommandFn fn;
};

#endif
