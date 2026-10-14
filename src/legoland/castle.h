#pragma once

/* [port] declared at file scope so prototypes below refer to the same type */
struct FVec3;

/* [port] file-scope forward declarations (structs first named in a prototype) */
struct BlokeInfo;
struct CallbackTable;
struct ClassNode;
struct HitHost;
struct PlaneSet;
#include "globals.h"

struct PathSeg;
struct CastleObj;
struct AreaNode;
struct Cursor;

/* a piece's opening: the direction mask and where it is relative to the piece */
struct PieceDoor {
    /* 0x00 */ unsigned int mask;
    /* 0x04 */ short x;
    /* 0x06 */ short y;
};

/* a piece's class (the piece's cls; the castle's own is DAT_004b5b48): data, then methods */
struct PieceClass {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ int f4;
    /* 0x08 */ int f8;
    /* 0x0c */ struct PieceDoor door_a;
    /* 0x14 */ struct PieceDoor door_b;
    /* 0x1c */ void (*method_1c)(struct PathSeg *self);
    /* 0x20 */ unsigned int (*method_20)(struct PathSeg *self, float height);
    /* 0x24 */ unsigned int (*method_24)(struct PathSeg *self);
    /* 0x28 */ void (*method_28)(struct PathSeg *self);
    /* 0x2c */ void (*method_2c)(struct PathSeg *self);
    /* 0x30 */ unsigned int field_30;
    /* 0x34 */ unsigned int field_34;
};

/* one end of a piece: the direction it opens to, its height and the piece joined there */
struct PieceEnd {
    /* 0x00 */ unsigned int dir;
    /* 0x04 */ float height;
    /* 0x08 */ struct PathSeg *link;
};

/* the start of every piece (and of the castle itself, CastleState.piece, the pieces' ring sentinel) */
struct PieceHead {
    /* 0x00 */ unsigned int flags;
    /* 0x04 */ union {
        struct Int16Pair pos;
        struct {
            short x;
            short y;
        };
        unsigned int xy; /* both coordinates at once, as a search key */
    };
    /* 0x08 */ struct Ride *ride;
    /* 0x0c */ struct PieceClass *cls;
    /* 0x10 */ struct CastleState *owner;
    /* 0x14 */ union {
        struct PieceEnd end_a;
        struct {
            unsigned int dir_a;
            float f18;
            struct PathSeg *prev;
        };
    };
    /* 0x20 */ union {
        struct PieceEnd end_b;
        struct {
            unsigned int dir_b;
            float f24;
            struct PathSeg *next;
        };
    };
};

struct Vec12 {
    unsigned int v[3];
};

struct SegBlob {
    /* 0x00 */ unsigned int v[17];
    /* 0x44 */ float t0;
    /* 0x48 */ float t1;
    /* 0x4c */ unsigned int w[3];
};

/* a castle piece (0xa4 bytes) */
struct PathSeg {
    struct PieceHead; /* MS extension: the header's fields are the piece's own */
    /* 0x2c */ struct Footprint fp; /* the piece's link of the castle cursor's footprint chain */
    /* 0x40 */ union {
        struct Vec12 vec;
        struct {
            unsigned int field_40;
            unsigned int field_44;
            unsigned int field_48;
        };
    };
    /* 0x4c */ struct SegBlob blob;
};

struct AreaNode {
    int x;
    int y;
    int w;
    int h;
    struct AreaNode *next;
};

struct DirPoints {
    unsigned int mask;
    short pt[4][2];
};

/* a visitor queueing for the castle (CastleState.visitors is the queue's sentinel) */
struct CastleVisitor {
    /* 0x00 */ unsigned int flags; /* 1, 2 or 4 */
    /* 0x04 */ unsigned char pad_4[4];
    /* 0x08 */ void *field_8;
    /* 0x0c */ unsigned char pad_c[4];
    /* 0x10 */ struct CastleVisitor *prev;
    /* 0x14 */ struct CastleVisitor *next;
    /* 0x18 */ unsigned int field_18;
    /* 0x1c */ unsigned int time;
};

/* DAT_00829ae0: the castle */
struct CastleState {
    /* 0x00 */ unsigned int flags; /* 2: the pieces form a closed ring */
    /* 0x04 */ struct PieceHead piece;
    /* 0x30 */ unsigned char pad_30[0xa8 - 0x30];
    /* 0xa8 */ struct PathSeg *f_a8;
    /* 0xac */ struct DirPoints pts_a;
    /* 0xc0 */ struct PathSeg *f_c0;
    /* 0xc4 */ struct DirPoints pts_b;
    /* 0xd8 */ struct CastleObj *field_d8;
    /* 0xdc */ unsigned char pad_dc[4];
    /* 0xe0 */ unsigned int field_e0;
    /* 0xe4 */ struct CastleVisitor visitors;
    /* 0x104 */ unsigned char pad_104[0x10c - 0x104];
    /* 0x10c */ void *method_10c; /* FUN_00424850: called through it by FUN_0041e500 */
    /* 0x110 */ void *method_110; /* FUN_00424890: FUN_0041e0e0 */
    /* 0x114 */ void *method_114; /* FUN_00424990: FUN_0041e100 */
};

struct Point;
int FUN_00424050(struct Point *pos, void *a2, void *a3, void *a4, void *a5);

struct FlagWord;
struct ObjAtC8;
struct ListLink;
struct Int16Pair;
struct EdgeMesh;
struct EdgeMesh *EdgeMeshRemoveBackfaces(struct EdgeMesh *src);
struct FVec3;
struct SprEnt;
struct Struct4274b0Src;
struct Struct4274b0Obj;
struct FitRes;
struct RingHost;
struct ObjAt40;
struct Elem20;
struct ObjE500;
struct RectI;
struct CastleOuter;
void FUN_00424a20(struct CastleOuter *param_1);
void FUN_004245b0(short *param_1);
void FUN_004249e0(struct CastleState *param_1);
void FUN_00424b10(struct CastleState *param_1);
struct RecIdx;
struct RecSrc;
void FUN_00423350(int n, struct RecIdx *idx, struct RecSrc *src);
struct FloatAtC4;
struct FloatHolder;
struct Struct426f40Dst;
struct Struct427070Src;
struct Struct427050Dst;
struct Struct426d80X;
struct SaveRec;
struct Struct426f40Src;
struct Struct426e80Dst;
struct BlokeSex0;
void FUN_00421980(struct CastleVisitor *p);

void FUN_00425cb0(const struct Int16Pair *in, float f, struct FVec3 *out);
void lego_invsqrtf_init(void);
void lego_sqrtf_init(void);
void FUN_00425bd0(void);
void FUN_00426740(void);
void FUN_00425d50(float *v);
float lego_sqrtf(float x);
unsigned int FUN_00420fb0(unsigned char *param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4);
void FUN_00426700(struct RectI *dst, struct RectI *src);
float lego_invsqrtf(float x);
float Vec3Dot(struct FVec3 *param_1, struct FVec3 *param_2);
void DrawDottedLine(void *a, void *b, short c);
void PlotClippedPoints(int *points, int count, short c);
void FUN_00423ec0(struct CastleState *castle);
void FUN_004292f0(struct FVec3 *pos, struct FVec3 *basis);
struct Struct428e70;
struct TexMesh;
struct TexMesh *FUN_00428cb0(struct Struct428e70 *obj, const struct FVec3 *offset, int fn, int count, const int *keys);
struct PathSeg;
unsigned int FUN_00429990(struct PathSeg *n, struct PathSeg **out);
void FUN_00429490(unsigned int param_1, unsigned int param_2);

struct Slot;
struct Struct42a5e0Obj;
struct Struct42a5e0Data;
struct SlotHost;
struct LmsFile;

unsigned int FUN_0041e720(struct SlotHost *host, unsigned int arg);
struct Slot *FUN_0041e760(struct SlotHost *host);
unsigned int FUN_0041e790(struct SlotHost *host);
void *FUN_0041eb30(unsigned int arg);
void FUN_0041eb60(unsigned int param_1);
void FUN_0041ed90(struct Element *element, int *pos);

unsigned int FUN_0041d1d0(struct PathSeg *obj, struct PieceDoor *door, struct DirPoints *pts);
float FUN_0041e7e0(unsigned char *obj);
float FUN_0041e810(struct FloatAtC4 *obj);
void FUN_0041e500(struct ObjE500 *obj);
void FUN_0041e630(struct FlagWord *obj);
void FUN_0041e6a0(unsigned int *param, unsigned int value);
void FUN_0041e950(struct ObjAtC8 *obj);
void FUN_0041e970(int param_1);
void FUN_0041e990(unsigned char *obj);
unsigned int FUN_0041eaf0(unsigned int a, unsigned int b);
unsigned int FUN_0041ee40(short *pos, struct AreaNode *area);
void FUN_0041ea70(unsigned int a);
void FUN_0042a680(unsigned char *p);
unsigned int FUN_00420e90(unsigned int a, unsigned int b, void *c, void *d, unsigned int e);
void FUN_00429af0(int a, void *b);
void FUN_004274b0(struct Struct4274b0Obj *obj, const struct Struct4274b0Src *src);
void FUN_0041f7f0(void);
unsigned int SetCurrentDirectoryIfNotNull(const char *param);
int LoadObjAndTxtFiles(const char *s);
void *ReadFileToBuffer(const char *fileName, unsigned int *bytesReadPtr);
unsigned int FUN_004225d0(void);
void FUN_004225b0(unsigned int param_1, char *param_2);
struct LmsFile *LoadLmsFile(const char *name);
int LoadLfmFile(const char *name, unsigned int *param2);
unsigned int FUN_00422640(void);
void FormatRollercoasterFileName(int value, char *buffer);
int LoadLtxFile(const char *name);
int LoadRollercoasterLpt(void);
void FUN_0041f030(unsigned int param);
int FUN_0041f4e0(void (*fn)(float, unsigned int), void *a, float b, float c, void *d);
int **FUN_0041f2b0(int n, int **verts, int *outCount, int nPlanes, float *planes);
void FUN_0041f790(float x, unsigned int out);
void FUN_00421470(void);
int FUN_0041f720(double (*fn)(float), float p2, float p3, float *out);
struct FitRes *FUN_0041f650(double (*fn)(float), int n, float a, float b);
double FUN_0041f7e0(float param);
void FUN_0041f710(unsigned int param);
float FUN_0041ddb0(unsigned char *param_1, float param_2);
void FUN_0041d950(void *obj, float val, void *tmp);
void FUN_0041db20(float a, float *out);
void FUN_0041da10(void *obj, float val, void *b);
unsigned int FUN_0041e930(struct ObjAt40 *obj, unsigned int param);
struct Struct42a110;
void FindPathPosAtRangeBehind(float *center, float r, struct Struct42a110 *src, float hi, float x, struct Struct42a110 *dst, float *out);
float CurveArcLength(struct Struct42a110 *a, unsigned int hi, struct Struct42a110 *b, unsigned int lo, int idx, float c);
void FUN_0041e7f0(unsigned char *obj, unsigned int *out);
int FUN_004298a0(struct PieceClass *info, struct CastleState *owner, struct PathSeg **a, struct PathSeg **b);
struct PathSeg *FUN_00429840(struct PathSeg *n, int x);
void FUN_00429750(struct PathSeg *seg, struct PathSeg *end);
void FUN_00424e60(struct CastleState *owner);
struct VecOps;
void FUN_00420310(struct VecOps *ar, float *p, float c);
void FUN_0042a620(unsigned int *param_1, unsigned int *param_2, unsigned int param_5);
void FUN_00424e70(struct CastleState *owner);
void FUN_004299e0(struct PathSeg *n);
void FUN_00429a30(struct PathSeg *n);
int FUN_0041d350(struct PieceClass *info, short *pos, struct SprEnt *ent);
int FUN_0041d2e0(struct PieceClass *info, short *pos, struct SprEnt *ent);
int FUN_0041d210(short *pos, struct SprEnt *ent, struct PieceClass *info);
void FUN_0041edb0(struct Element *element, TileId tile, struct Cursor *cursor);
void FUN_0041d760(struct PathSeg *obj);
float FUN_0041dca0(unsigned char *param);
void FUN_0041db90(unsigned char *obj, float *a, float *b);
struct PathSeg *FUN_0041d100(struct CastleState *host, struct Ride *ride, short *pt);
void FUN_0041e9e0(unsigned char *obj, float *out);
void FUN_0041ef20(int a, int b, int c, int d);
void FUN_0041f380(const int *r, struct PlaneSet *out);
float FUN_0041dd00(unsigned char *param, float result);
float FUN_0041dd50(unsigned char *param);
void FUN_0041e8f0(unsigned char *obj, struct Elem20 *e, float t);
void FUN_00429c60(void *obj, int a, unsigned int b, float c, struct FVec3 *out);
void FUN_0042a5e0(struct Struct42a5e0Obj *o, struct Struct42a5e0Data *src, float param_5);
void FUN_0041e820(unsigned char *obj, struct Elem20 *e, float t);
void FUN_00420410(void *param, unsigned int count);
unsigned int GetLmsByName(const char *s);
unsigned int GetLfmByName(const char *s);
void **FUN_004214f0(unsigned int param_1);
void FUN_00421510(unsigned int param_1, unsigned int param_2);
void FUN_00421540(void *dest, unsigned int value);
unsigned int FUN_00422590(const char *s);
unsigned int FUN_00422600(unsigned int param);
void FUN_004220e0(float *obj, float t, float *out);
void FUN_00422e10(int param1, int index);
void FUN_00422e40(int param1, char *entry);
void CastleObjLoad(struct Element *param_1);
void FUN_00424280(struct Element *param_1, int param_2, unsigned int param_3);
void FUN_004244b0(unsigned int param_1, TileId param_2, unsigned int param_3);
void FUN_00424620(struct Int16Pair *pos);
void FUN_00424800(void);
void FUN_00424850(unsigned char *p1, unsigned int *p2, unsigned int *p3, struct FVec3 *p4);
float FUN_00424890(unsigned char *param_1, float param_2);
struct CastleActor;
float FUN_00424990(struct CastleActor *self, float value);
void FUN_00424a50(struct CastleState *param_1);
void FUN_00424a00(struct CastleState *param_1);
void FUN_00424ab0(struct CastleState *this);
void FUN_00424d80(struct CastleState *list);
void FUN_00424df0(struct CastleState *esi);
void CastleRemoveAllRiders(void);
void FUN_00425a50(void);
void FUN_00425c40(void);
void Mat4Identity(float m[4][4]);
void Mat4Multiply(float a[4][4], float b[4][4], float out[4][4]);
unsigned int FUN_00426230(float in[][3], int out[][4], int n);
unsigned int FUN_00426250(float in[][3], int out[][4], float *m, unsigned int stride, int n);
unsigned int FUN_00426650(void);
void FUN_004266b0(struct ListLink *param_1);
void UnlinkListLink(struct ListLink *param_1);
unsigned int FUN_00426750(void *ptr, unsigned int a, unsigned int b, unsigned int c);
unsigned int FUN_00427130(struct CastleState *main);
unsigned int FUN_00427150(struct CastleState *arg);
void FUN_00428b70(void);
void FUN_004284d0(void);
void FUN_00428750(void);
void FUN_0042a2e0(void);
void LoadCoasterTrainWheelModel(void);
void FUN_004273d0(unsigned int param_1, void *param_2);
unsigned int FUN_004273e0(void *obj);
void FUN_00428700(void *node);
void FUN_004294f0(unsigned int a, unsigned int *b, unsigned int c, unsigned int d);
void FUN_00428f00(void);
void FUN_00429270(void);
float FUN_0042a150(unsigned int b);
void FUN_004294b0(void);
unsigned int FUN_00429910(struct PieceClass *s, unsigned int v, unsigned int c);
unsigned int FUN_00429940(struct PathSeg *n, struct PathSeg **out);
unsigned int FUN_0042a640(void *ptr, unsigned int a, unsigned int b);

void CastleGetInterfaces(struct ClassNode *head, struct CallbackTable *iface);
struct Element;
struct Struct427970Src;
struct Struct427bc0Src;
struct CastleRideObj;
int CastleObj_Load(void);
int CastleObj_Save(void);
void SquareTrackCalcCursor(struct Element *obj, int x, unsigned int y);
struct PieceClass *FUN_00427c00(struct Element *element);
void FUN_00427940(struct Element *param_1);
void FUN_00427970(struct Element *obj, const struct Struct427970Src *src);
void FUN_004279f0(void);
void RenderSquareTrack(unsigned int unused0, unsigned int unused1, unsigned int unused2, const unsigned char *src);
void CastleLoadBasicTiles(struct Element *obj);
void CastleUnloadBasicTiles(void);
void FUN_00427b20(struct Element *obj, int x, unsigned int y);
void FUN_00427bc0(unsigned int unused, const struct Struct427bc0Src *src);
void FUN_00427c90(struct Element *obj, int x, unsigned int y);
void FUN_00427ea0(struct Element *obj, const struct Struct427bc0Src *src);
void SquareTrackHeightLoad(struct Element *param_1);
void SquareTrackHeight0Load(struct Element *param_1);
void SquareTrackHeightPathLoad(struct Element *param_1);
void FUN_004280b0(struct Element *obj, int x, unsigned int y);
void FUN_00428300(struct Element *obj, const struct Struct427bc0Src *src);
void FUN_00425e20(void);

struct SaveRec *ReadSaveRec(void);
void FUN_00426f90(struct Struct426f40Dst *src, struct CastleState *obj);
void FUN_00427100(struct Struct427070Src *arr, int count, struct CastleState *obj);
void FUN_00426d80(struct CastleState *param1, struct Struct426d80X *param2);
void FUN_00426de0(struct Struct426d80X *src, struct SaveRec *dst);
void FUN_004270c0(struct CastleState *a1, struct Struct427050Dst *a2);
void WriteSaveRec(unsigned int *param_1);
void FUN_00427190(struct CastleState *obj, struct Struct426e80Dst *out);
void FUN_00426f40(struct Struct426f40Src *src, struct Struct426f40Dst *dst);
unsigned int FUN_00421660(struct BlokeInfo *p);

struct BoxFace {
    short f0;
    short plane;
    short t[3];
    short u[3];
};

typedef struct BoxFace BoxFace;

struct BoxSolid {
    int f0;
    int f4;
    int f8;
    struct FVec3 *verts;
    float (*normals)[3];
    struct FVec3 *xverts;
    struct BoxFace *faceA;
    int nFaceA;
    struct BoxFace *faceB;
    int nFaceB;
    unsigned char pad_28[0x90 - 0x28];
};

extern float DAT_004b5700[6][3];
extern struct BoxFace DAT_004b5748[12];
extern struct BoxFace DAT_004b5808[12];
extern struct BoxSolid DAT_004b58c8;

void FUN_00420fd0(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d);
void FUN_00421130(struct BoxSolid *box, struct FVec3 *verts, struct FVec3 *xverts, int unused, float w, float h, float d);
struct RecBuf;
struct ClearRect;
void FUN_004232b0(struct RecBuf *rb);
void FUN_00423480(struct ClearRect *r);
struct AnimWalker;
unsigned int FUN_0041f850(struct AnimWalker *cursor);
struct VideoArg;
int FUN_00423760(struct VideoArg *arg);
void FUN_00423790(void);
void FUN_004261c0(float *in, float *out, float m[4][4], int n);
void FUN_004264e0(unsigned int *t, unsigned int *m3, unsigned int dst[4][4]);
struct Mat4x4;
void FUN_00426510(unsigned int *m3, struct Mat4x4 *out);
struct RecIdx;
struct RecSrc;
void FUN_0041f8d0(int palette, int *color_index, int n, struct RecIdx *idx, struct RecSrc *src);
void FUN_0041fa10(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);
void FUN_0041fba0(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);
void FUN_0041fd80(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);
void FUN_0041ff80(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);
int FUN_004234e0(void *param1);
struct PolyArg;
void FUN_0042a2f0(int n, struct PolyArg *poly);
void FUN_00428860(int palette, int *shade, int n, struct RecIdx *idx, struct RecSrc *src);
void BuildBasisFromDir(struct FVec3 *dir, struct FVec3 *basis);
unsigned int FUN_00428840(unsigned int param_1);
unsigned int GetLtxFileTableEntry(unsigned int param);
