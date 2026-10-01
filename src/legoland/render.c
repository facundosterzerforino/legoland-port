#include <windows.h>
#include <ddraw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "legoland.h"

#include "../../port/port_asm.h"
#include "challenge.h"
#include "draw.h"
#include "gfx.h"
#include "globals.h"
#include "image_sprite.h"
#include "print_sprite.h"
#include "render.h"

struct ZBlitDesc {
    int off[2];
    RECT rect;
};

struct CursorCacheNode {
    struct CursorCacheNode *next;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char pad_7;
    unsigned int value;
};

struct CursorKey {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct CursorBitmap {
    unsigned char pad_0[4];
    void *pixels;
};

struct RenderViewport {
    unsigned int x;
    unsigned int y;
};

struct TextureFrame {
    /* 0x00 */ void *field_0;
    /* 0x04 */ unsigned short *data;
};

struct TextureDesc {
    /* 0x00 */ unsigned int width;
    /* 0x04 */ unsigned int height;
    /* 0x08 */ unsigned char shift;
    /* 0x09 */ unsigned char pad_9[3];
    /* 0x0c */ unsigned char *index_map;
    /* 0x10 */ struct TextureFrame **frame_table;
};

// FUNCTION: LEGOLAND 0x004860f0
void FUN_004860f0(void) {
    int i;
    unsigned int n;
    unsigned int sh;
    unsigned int mask;
    unsigned short v;
    float f;

    n = *(unsigned int *)&DAT_007cb5e0;
    sh = n + 5;
    mask = 0xffu >> (8 - n);
    for (i = 0; i < 256; i++) {
        f = i * FLOAT_004ab550;
        v = (unsigned short)(int)(f * FLOAT_004ab444);
        DAT_0066b638[i].shifted = v << sh;
        DAT_0066b638[i].scaled = (int)(f * mask) << 5;
        DAT_0066b638[i].value = v;
    }
}

// FUNCTION: LEGOLAND 0x00486190
unsigned int FUN_00486190(struct CursorKey *key) {
    struct CursorCacheNode *node;

    if (DAT_00797e6c != 0) {
        node = DAT_00797e6c;
        while (node != 0) {
            if (node->b == key->b && node->g == key->g && node->r == key->r) {
                return node->value;
            }
            node = node->next;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004861d0
struct CursorCacheNode *FUN_004861d0(unsigned int value, const unsigned char *key) {
    struct CursorCacheNode *node = (struct CursorCacheNode *)malloc(0xc);
    if (node != 0) {
        memset(node, 0, 0xc);
        node->b = key[2];
        node->g = key[1];
        node->r = key[0];
        node->value = value;
        if (DAT_00797e6c) {
            node->next = DAT_00797e6c;
        }
        DAT_00797e6c = node;
    }
    return node;
}

// FUNCTION: LEGOLAND 0x00486220
void FUN_00486220(void *param) {
    struct CursorBitmap *bmp = (struct CursorBitmap *)param;
    if (bmp != 0) {
        if (bmp->pixels != 0) {
            free(bmp->pixels);
        }
        free(bmp);
    }
}

// FUNCTION: LEGOLAND 0x00486250
void FUN_00486250(void) {
    struct CursorCacheNode *node = DAT_00797e6c;
    while (node != 0) {
        struct CursorCacheNode *next = node->next;
        FUN_00486220((void *)node->value);
        free(node);
        node = next;
    }
}

// FUNCTION: LEGOLAND 0x00486280
unsigned int FUN_00486280(int param_1, void *param_2) {
    struct CursorKey *key = (struct CursorKey *)param_2;
    struct TextureFrame *frame;
    unsigned int cached;
    float fb, fg, fr;
    float sb, sg, sr;
    float ab, ag, ar;
    int i;

    cached = FUN_00486190(key);
    if (cached != 0) {
        return cached;
    }
    frame = (struct TextureFrame *)malloc(8);
    if (frame != 0) {
        frame->field_0 = (void *)param_1;
        frame->data = (unsigned short *)malloc(param_1 * 2);
        fb = key->b;
        fg = key->g;
        fr = key->r;
        param_1 >>= 1;
        ab = FLOAT_004ab390;
        ag = FLOAT_004ab390;
        ar = FLOAT_004ab390;
        sb = fb / param_1;
        sg = fg / param_1;
        sr = fr / param_1;
        i = 0;
        if (param_1 > 0) {
            do {
                frame->data[i++] = DAT_0066b638[(unsigned char)(int)ar].value | DAT_0066b638[(unsigned char)(int)ag].scaled | DAT_0066b638[(unsigned char)(int)ab].shifted;
                ab += sb;
                ag += sg;
                ar += sr;
            } while (i < param_1);
        }
        sb = (255.0f - fb) / param_1;
        sg = (255.0f - fg) / param_1;
        sr = (255.0f - fr) / param_1;
        ab = fb;
        ag = fg;
        ar = fr;
        if (param_1 > 0) {
            i = param_1;
            do {
                frame->data[i++] = DAT_0066b638[(unsigned char)(int)ar].value | DAT_0066b638[(unsigned char)(int)ag].scaled | DAT_0066b638[(unsigned char)(int)ab].shifted;
                ab += sb;
                ag += sg;
                ar += sr;
            } while (--param_1 != 0);
        }
    }
    FUN_004861d0((unsigned int)frame, (unsigned char *)key);
    return (unsigned int)frame;
}

// FUNCTION: LEGOLAND 0x004864e0
void FUN_004864e0(unsigned int param_1) {
    DAT_0066b61c = param_1;
}

// FUNCTION: LEGOLAND 0x00486540
int FUN_00486540(void) {
    int i;
    float f;

    for (i = 1; i < 100; i++) {
        f = (float)i;
        DAT_00797e70[i] = 1.0f / f;
        DAT_00798000[i] = (int)(DOUBLE_004ab558 / f);
    }
    return 0;
}

/* Port [library:asm]: the original has four near-identical triangle fillers in inline asm (FUN_00486590, FUN_00486c70,
 * FUN_004877b0, FUN_00487d40). They share one scan converter, PortRasterTriangle, which is parameterised by
 * a per-pixel colour function.
 *
 * Every vertex carries x, y (16.16 fixed point) and a few more values that are interpolated along the edges
 * and then along each scanline: v[0] = x, v[1] = depth, v[2..] = shading level and/or texture coordinates.
 * The triangle is split at the middle vertex into two parts (a flat-top triangle has only the second).
 * A pixel is drawn only if its depth is >= the depth already in the z-buffer (unsigned compare); drawing a
 * pixel also sets DAT_007feb14 when it is the pixel under the mouse (DAT_007fe9a8).
 *
 * Quirks kept from the original: the z-buffer address is zbuf + y * 512 + x * 4 (rows 512 bytes apart,
 * although a sprite is up to 160 pixels = 640 bytes wide); the reciprocal table DAT_00798000 only has
 * entries 1..99 (the original reads past it for taller triangles, here the index is clamped). */
#define PORT_NV 5

struct PortVert {
    int y;
    unsigned int v[PORT_NV];
};

struct PortEdge {
    unsigned int v[PORT_NV];
    unsigned int d[PORT_NV]; /* step per scanline */
};

/* Returns the 16-bit colour for a pixel; s[1] is the depth, s[2..] the other interpolated values. */
typedef unsigned short (*PortPixelFn)(const unsigned int *s, const void *ctx);

static __inline int PortRecip(int n) {
    if (n > 99) {
        n = 99;
    }
    return DAT_00798000[n];
}

static void PortEdgeInit(struct PortEdge *e, const struct PortVert *from, const struct PortVert *to, int inv, int nv) {
    int i;

    for (i = 0; i < nv; i++) {
        e->v[i] = from->v[i];
        e->d[i] = PortFixMul((int)(to->v[i] - from->v[i]), inv);
    }
}

/* Draws scanlines from *py up to (not including) yend, advancing both edges. */
static void PortScanLines(struct PortEdge *e1, struct PortEdge *e2, int *py, unsigned int *prow, int yend, int nv, PortPixelFn pixel, const void *ctx) {
    int y = *py;
    unsigned int row = *prow;
    int n;
    int i;

    /* skip the lines above the clip rectangle */
    n = (int)DAT_0081c8d4 - y;
    if (n > 0 && yend - y > 0) {
        if (n > yend - y) {
            n = yend - y;
        }
        y += n;
        for (i = 0; i < nv; i++) {
            e1->v[i] += e1->d[i] * n;
            e2->v[i] += e2->d[i] * n;
        }
        row += DAT_00701e58 * n;
    }
    while (y < yend && y <= (int)DAT_0081c8dc) {
        const struct PortEdge *lo;
        const struct PortEdge *hi;
        unsigned int s[PORT_NV];
        unsigned int ds[PORT_NV];
        unsigned int rowaddr = row;
        unsigned int p;
        unsigned int plimit;
        unsigned int *zp;
        int inv;
        int x;
        int xend;

        row += DAT_00701e58;
        if ((int)e1->v[0] > (int)e2->v[0]) {
            lo = e2;
            hi = e1;
        } else {
            lo = e1;
            hi = e2;
        }
        inv = PortRecip((int)((hi->v[0] - lo->v[0]) >> 16) + 1);
        for (i = 1; i < nv; i++) {
            s[i] = lo->v[i];
            ds[i] = PortFixMul((int)(hi->v[i] - lo->v[i]), inv);
        }
        x = (int)lo->v[0] >> 16;
        xend = (int)hi->v[0] >> 16;
        if (x < (int)DAT_0081c8d0) {
            n = (int)DAT_0081c8d0 - x;
            x = (int)DAT_0081c8d0;
            for (i = 1; i < nv; i++) {
                s[i] += ds[i] * n;
            }
        }
        p = rowaddr + x * 2;
        plimit = rowaddr + DAT_0081c8d8 * 2;
        while (x < xend && p <= plimit) {
            zp = (unsigned int *)(DAT_00701e5c + (y << 9) + x * 4);
            if (s[1] >= *zp) {
                *zp = s[1];
                DAT_007feb14 |= (p == DAT_007fe9a8);
                *(unsigned short *)p = pixel(s, ctx);
            }
            x++;
            for (i = 1; i < nv; i++) {
                s[i] += ds[i];
            }
            p += 2;
        }
        for (i = 0; i < nv; i++) {
            e1->v[i] += e1->d[i];
            e2->v[i] += e2->d[i];
        }
        y++;
    }
    *py = y;
    *prow = row;
}

static void PortRasterTriangle(struct PortVert *a, struct PortVert *b, struct PortVert *c, int nv, PortPixelFn pixel, const void *ctx) {
    struct PortVert *t;
    struct PortEdge e1;
    struct PortEdge e2;
    int ya;
    int yb;
    int yc;
    int y;
    int i;
    int inv;
    unsigned int row;

    /* sort the vertices by y (a on top) */
    if (a->y > b->y) {
        t = a;
        a = b;
        b = t;
    }
    if (b->y > c->y) {
        t = b;
        b = c;
        c = t;
    }
    if (a->y > b->y) {
        t = a;
        a = b;
        b = t;
    }
    ya = a->y >> 16;
    yb = b->y >> 16;
    yc = c->y >> 16;
    y = ya;
    row = DAT_00797e68 + DAT_00701e58 * ya;
    if (ya != yb) {
        /* first part: edges a-b and a-c down to the middle vertex */
        PortEdgeInit(&e1, a, b, PortRecip(yb - ya), nv);
        PortEdgeInit(&e2, a, c, PortRecip(yc - ya), nv);
        PortScanLines(&e1, &e2, &y, &row, yb, nv, pixel, ctx);
        /* the second part carries on from where the first left off; only the b-c slopes are new */
        inv = PortRecip(yc - yb);
        for (i = 0; i < nv; i++) {
            e1.d[i] = PortFixMul((int)(c->v[i] - b->v[i]), inv);
        }
    } else {
        /* flat top: edges a-c and b-c */
        PortEdgeInit(&e1, a, c, PortRecip(yc - ya), nv);
        PortEdgeInit(&e2, b, c, PortRecip(yc - yb), nv);
    }
    PortScanLines(&e1, &e2, &y, &row, yc, nv, pixel, ctx);
}

/* shaded solid colour: the colour set (a TextureFrame) indexed by the high half of the shade value s[2] */
static unsigned short PortShadedPixel(const unsigned int *s, const void *ctx) {
    const struct TextureFrame *frame = (const struct TextureFrame *)ctx;

    return frame->data[s[2] >> 16];
}

/* one fixed colour */
static unsigned short PortFlatPixel(const unsigned int *s, const void *ctx) {
    (void)s;
    return *(const unsigned short *)ctx;
}

/* Textured, shade interpolated (s[2]), u = s[3], v = s[4] (16.16, already shifted by the texture's log2
 * width / height): the high halves pick a texel, whose value selects a colour set (TextureFrame) that the
 * high half of the shade indexes. The row mask is width_m1 and the column mask height_m1, as in the
 * original (equivalent for the square textures). */
static unsigned short PortTexturedPixel(const unsigned int *s, const void *ctx) {
    const struct TextureNode *tex = (const struct TextureNode *)ctx;
    unsigned int row = ((s[4] >> 16) & tex->width_m1) << tex->format_w;
    unsigned int col = (s[3] >> 16) & tex->height_m1;
    const struct TextureFrame *frame = (const struct TextureFrame *)tex->data_c[tex->data_8[row + col]];

    return frame->data[s[2] >> 16];
}

/* Textured with a constant shade: u = s[2], v = s[3]; no masks. */
struct PortTexCtx {
    const struct TextureNode *tex;
    unsigned int shade;
};

static unsigned short PortTexturedFlatPixel(const unsigned int *s, const void *ctx) {
    const struct PortTexCtx *tc = (const struct PortTexCtx *)ctx;
    const struct TextureNode *tex = tc->tex;
    unsigned int row = (s[3] >> 16) << tex->format_w;
    unsigned int col = s[2] >> 16;
    const struct TextureFrame *frame = (const struct TextureFrame *)tex->data_c[tex->data_8[row + col]];

    return frame->data[tc->shade >> 16];
}

// FUNCTION: LEGOLAND 0x00486590
void FUN_00486590(struct PersonVertex *a, struct PersonVertex *b, struct PersonVertex *c) {
    /* Port [library:asm]: the original is inline asm. Fills a Gouraud-shaded solid-colour triangle into the
     * 16-bit surface with z-buffering. The shade (vertex shade * 64) is interpolated and indexes the
     * colour set chosen with FUN_004864e0 (DAT_0066b61c). */
    struct PortVert v[3];
    struct PersonVertex *in[3];
    int i;

    in[0] = a;
    in[1] = b;
    in[2] = c;
    for (i = 0; i < 3; i++) {
        v[i].y = in[i]->y;
        v[i].v[0] = in[i]->x;
        v[i].v[1] = in[i]->depth;
        v[i].v[2] = (unsigned int)in[i]->shade << 6;
    }
    PortRasterTriangle(&v[0], &v[1], &v[2], 3, PortShadedPixel, (const void *)DAT_0066b61c);
}

// FUNCTION: LEGOLAND 0x00486c70
void FUN_00486c70(struct PersonVertex *a, struct PersonVertex *b, struct PersonVertex *c) {
    /* Port [library:asm]: the original is inline asm. Fills a textured, Gouraud-shaded triangle (texture
     * chosen with FUN_00485f20 / DAT_0066b630) with z-buffering; u, v and the shade are interpolated. */
    struct PortVert v[3];
    struct PersonVertex *in[3];
    const struct TextureNode *tex = (const struct TextureNode *)DAT_0066b630;
    int i;

    in[0] = a;
    in[1] = b;
    in[2] = c;
    for (i = 0; i < 3; i++) {
        v[i].y = in[i]->y;
        v[i].v[0] = in[i]->x;
        v[i].v[1] = in[i]->depth;
        v[i].v[2] = (unsigned int)in[i]->shade << 6;
        v[i].v[3] = ((unsigned int)PortRound(in[i]->u * 65536.0f) & 0xffff) << tex->format_w;
        v[i].v[4] = ((unsigned int)PortRound(in[i]->v * 65536.0f) & 0xffff) << tex->format_h;
    }
    PortRasterTriangle(&v[0], &v[1], &v[2], 5, PortTexturedPixel, tex);
}

// FUNCTION: LEGOLAND 0x004877b0
void FUN_004877b0(struct PersonVertex *a, struct PersonVertex *b, struct PersonVertex *c) {
    /* Port [library:asm]: the original is inline asm. Like FUN_00486590 but flat shaded: the whole triangle
     * gets the colour picked by the first vertex's shade (the original also stores that vertex's shade * 64
     * back into the caller's vertex). */
    struct PortVert v[3];
    struct PersonVertex *in[3];
    const struct TextureFrame *frame = (const struct TextureFrame *)DAT_0066b61c;
    unsigned short color;
    int i;

    in[0] = a;
    in[1] = b;
    in[2] = c;
    a->shade = a->shade << 6;
    color = frame->data[(unsigned int)a->shade >> 16];
    for (i = 0; i < 3; i++) {
        v[i].y = in[i]->y;
        v[i].v[0] = in[i]->x;
        v[i].v[1] = in[i]->depth;
    }
    PortRasterTriangle(&v[0], &v[1], &v[2], 2, PortFlatPixel, &color);
}

// FUNCTION: LEGOLAND 0x00487d40
void FUN_00487d40(struct PersonVertex *a, struct PersonVertex *b, struct PersonVertex *c) {
    /* Port [library:asm]: the original is inline asm. Like FUN_00486c70 but flat shaded with the first
     * vertex's shade; u and v are interpolated. */
    struct PortVert v[3];
    struct PersonVertex *in[3];
    struct PortTexCtx ctx;
    int i;

    in[0] = a;
    in[1] = b;
    in[2] = c;
    ctx.tex = (const struct TextureNode *)DAT_0066b630;
    ctx.shade = (unsigned int)a->shade << 6;
    for (i = 0; i < 3; i++) {
        v[i].y = in[i]->y;
        v[i].v[0] = in[i]->x;
        v[i].v[1] = in[i]->depth;
        v[i].v[2] = ((unsigned int)PortRound(in[i]->u * 65536.0f) & 0xffff) << ctx.tex->format_w;
        v[i].v[3] = ((unsigned int)PortRound(in[i]->v * 65536.0f) & 0xffff) << ctx.tex->format_h;
    }
    PortRasterTriangle(&v[0], &v[1], &v[2], 4, PortTexturedFlatPixel, &ctx);
}

// FUNCTION: LEGOLAND 0x00488670
void FUN_00488670(struct Image *image, unsigned int index) {
    struct TextureNode *ptr = (struct TextureNode *)malloc(sizeof(struct TextureNode));
    if (ptr != 0) {
        FUN_004437d0(image, ptr);
        DAT_00798190[index] = ptr;
    }
}

// FUNCTION: LEGOLAND 0x004886a0
void FUN_004886a0(void) {
    struct TextureNode **slot = (struct TextureNode **)&DAT_00798190;
    do {
        if (*slot != 0) {
            free((*slot)->data_8);
            free((*slot)->data_c);
            free(*slot);
            *slot = 0;
        }
        slot++;
    } while ((int)slot < (int)&DAT_00798590);
}

// FUNCTION: LEGOLAND 0x004886e0
void FUN_004886e0(unsigned int index) {
    FUN_00485f20(DAT_00798190[index]);
}

// FUNCTION: LEGOLAND 0x00488700
void FUN_00488700(unsigned int base, struct RenderViewport *vp) {
    DAT_007feb14 = 0;
    DAT_007fe9a8 = base + (vp->y * DAT_00701e58) + (vp->x * 2);
}

// FUNCTION: LEGOLAND 0x00488730
unsigned short FUN_00488730(unsigned int p1, unsigned int p2, unsigned int p3) {
    struct TextureDesc *desc = (struct TextureDesc *)DAT_0066b630;
    unsigned int u = (unsigned int)((unsigned __int64)(desc->width - 1) * (p1 & 0xffff) >> 0x10);
    unsigned int v = (unsigned int)((unsigned __int64)(desc->height - 1) * (p2 & 0xffff) >> 0x10);
    unsigned int idx = (v << desc->shift) + u;
    struct TextureFrame *frame = desc->frame_table[desc->index_map[idx]];
    return frame->data[(p3 - ((p3 >> 0x10 & 1) != 0)) >> 10];
}

// FUNCTION: LEGOLAND 0x004887a0
void FUN_004887a0(void) {
    DAT_00798590 = (void *)1;
    DAT_0066be50 = CreateSourceImage(DAT_004d8bb0, 0);
    DAT_0066be50->data = DAT_00701e68;
    DAT_0066be50->width = 0x280;
    DAT_0066be50->height = 0x1e0;
    DAT_0066be50->refcount = 1;
    DAT_0066be50->aux = NULL;
    DAT_0066be50->field_14 = 1;
    DAT_00701e64 = CreateSprite(DAT_0066be50);
    DAT_00701e64->flags = DAT_00701e64->flags | 0x208;
}

// FUNCTION: LEGOLAND 0x00488820
unsigned int FUN_00488820(unsigned int x, unsigned int y) {
    unsigned int *row = (unsigned int *)(DAT_00701e5c + DAT_0066be4c * y);
    return row[x] >> 0x18;
}

// FUNCTION: LEGOLAND 0x00488840
LEGO_EXPORT struct Sprite *GenerateNewImageFromZBuffer(struct Sprite *sprite, struct Sprite *param_2, int param_3, int param_4, int param_5) {
    int w = (short)sprite->width;
    int h = (short)sprite->height;
    struct ZBlitDesc local;
    unsigned int transp;
    int row;
    int yy;

    if (DAT_00798590 == 0) {
        FUN_004887a0();
    }
    DAT_0066b620 = DAT_00668108.left;
    DAT_0066b628 = DAT_00668108.right;
    DAT_0066b5b0 = DAT_0066809c;
    DAT_0066b62c = DAT_00668108.bottom;
    DAT_0066b624 = DAT_00668108.top;
    DAT_0066809c.lpSurface = DAT_0066be54;
    DAT_0066809c.dwWidth = w;
    DAT_0066809c.dwHeight = h;
    DAT_0066809c.lPitch = w * 2;
    DAT_00668108.left = 0;
    DAT_00668108.right = w;
    DAT_00668108.top = 0;
    DAT_00668108.bottom = h;
    local.off[0] = 0;
    local.off[1] = 0;
    local.rect.left = 0;
    local.rect.right = w;
    local.rect.top = 0;
    local.rect.bottom = h;
    DAT_007fea44 = GetTransparentColour();
    SoftPrint_Clear();
    FUN_00464ee0(param_2, &local.rect, local.off);
    FUN_00485fe0(param_2, param_4, param_5);
    transp = GetTransparentColour();
    yy = 0;
    if (0 < h) {
        row = 0;
        do {
            int xx = 0;
            int col = row;
            if (0 < w) {
                do {
                    unsigned int z = FUN_00488820(xx, yy);
                    unsigned short px = transp;
                    if ((int)(z & 0xff) <= param_3) {
                        px = *(unsigned short *)((char *)DAT_0066be54 + col);
                    }
                    *(unsigned short *)((char *)DAT_00701e68 + col) = px;
                    xx = xx + 1;
                    col = col + 2;
                } while (xx < w);
            }
            yy = yy + 1;
            row = row + w * 2;
        } while (yy < h);
    }
    DAT_0066be50->width = (short)w;
    DAT_0066be50->height = (short)h;
    DAT_00701e64->width = (short)w;
    DAT_00701e64->height = (short)h;
    DAT_00668108.right = DAT_0066b628;
    DAT_00668108.top = DAT_0066b624;
    DAT_0066809c = DAT_0066b5b0;
    DAT_00668108.left = DAT_0066b620;
    DAT_00668108.bottom = DAT_0066b62c;
    return DAT_00701e64;
}

// FUNCTION: LEGOLAND 0x00488a10
LEGO_EXPORT unsigned int RenderSprite(struct Sprite *sprite, int x, int y) {
    RECT dst;
    RECT src;
    DDBLTFX fx;

    dst.left = x;
    dst.top = y;
    dst.right = (short)sprite->width + x;
    dst.bottom = (short)sprite->height + y;
    src.left = 0;
    src.top = 0;
    src.bottom = (short)sprite->height;
    src.right = (short)sprite->width;
    if ((sprite->flags & 0x60) != 0) {
        if (IntersectRect(&dst, &dst, &SPRITE_ClipRect) != 0) {
            int i;
            unsigned int *p;
            sprite->field_c = DAT_008119a4;
            src.left = dst.left;
            src.top = dst.top;
            src.right = dst.right;
            src.bottom = dst.bottom;
            OffsetRect(&src, -x, -y);
            p = (unsigned int *)&fx;
            for (i = 0x19; i != 0; i--) {
                *p = 0;
                p++;
            }
            fx.dwSize = 100;
            PushRenderingStatusAndUnlockVideoSurface();
            if ((sprite->flags & 0x40) != 0) {
                ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->Blt((LPDIRECTDRAWSURFACE)renderEngine, &dst, (LPDIRECTDRAWSURFACE)sprite->surface, &src, 0x1008000, &fx);
            } else {
                ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->Blt((LPDIRECTDRAWSURFACE)renderEngine, &dst, (LPDIRECTDRAWSURFACE)sprite->surface, &src, 0x1000000, &fx);
            }
            PopRenderingStatus();
            return 1;
        }
    } else {
        if (IntersectRect(&dst, &dst, &SPRITE_ClipRect) != 0) {
            sprite->field_c = DAT_008119a4;
            src.left = dst.left;
            src.top = dst.top;
            src.right = dst.right;
            src.bottom = dst.bottom;
            OffsetRect(&src, -x, -y);
            FUN_00464ee0(sprite, &src, (int *)&dst);
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00488b90
LEGO_EXPORT unsigned int RenderSpriteX(struct Sprite *sprite, int x, int y, unsigned int param_4) {
    RECT dst;
    RECT src;

    dst.left = x;
    dst.top = y;
    dst.right = (short)sprite->width + x;
    dst.bottom = (short)sprite->height + y;
    src.left = 0;
    src.top = 0;
    src.bottom = (short)sprite->height;
    src.right = (short)sprite->width;
    if (IntersectRect(&dst, &dst, &SPRITE_ClipRect) != 0) {
        sprite->field_c = DAT_008119a4;
        src.left = dst.left;
        src.top = dst.top;
        src.right = dst.right;
        src.bottom = dst.bottom;
        OffsetRect(&src, -x, -y);
        SoftPrint_XBltFast(sprite, &src, &dst, param_4);
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x00488c50
// Never implemented: it exits before drawing. The Blt after exit(1) is a
// reconstruction; the compiler drops it as unreachable, but because it takes
// &dst the four stores into dst survive, exactly as in the original.
LEGO_EXPORT unsigned int RenderTiledSprite(struct Sprite *sprite, int param_2, int param_3, int param_4, int param_5, int param_6, int param_7) {
    RECT dst;

    dst.left = param_2;
    dst.top = param_3;
    dst.right = param_2 + param_4;
    dst.bottom = param_3 + param_5;
    exit(1);
    return ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->Blt((LPDIRECTDRAWSURFACE)renderEngine, &dst, (LPDIRECTDRAWSURFACE)sprite->surface, NULL, 0x1000000, NULL) == 0;
}

// FUNCTION: LEGOLAND 0x00488c80
unsigned int FUN_00488c80(struct Sprite *sprite, int param_2, int param_3, int param_4, int param_5, int *param_6) {
    int off[2];
    RECT src;
    RECT dst;
    RECT clip;
    RECT r2;
    DDSURFACEDESC desc1;
    DDSURFACEDESC desc2;
    HRESULT hr;

    int x = param_2 + param_6[0];
    int y = param_3 + param_6[1];
    dst.left = x;
    dst.top = y;
    dst.right = x + param_4 + 1;
    dst.bottom = y + param_5 + 1;
    src.top = 0;
    src.left = 0;
    src.right = (short)sprite->width;
    src.bottom = (short)sprite->height;
    if (DAT_00798620 == 0) {
        memset(&desc1, 0, sizeof(desc1));
        DAT_00798620 = 1;
        desc1.dwSize = 0x6c;
        if (IDirectDrawSurface_GetSurfaceDesc(renderEngine, &desc1) == 0) {
            desc1.dwFlags = 0x1007;
            desc1.ddsCaps.dwCaps = 0x840;
            desc1.dwWidth = 0x500;
            desc1.dwHeight = 0x3c0;
            IDirectDraw2_CreateSurface(DDRAWENV.ddraw2, &desc1, &DAT_0079861c, NULL);
            off[1] = off[0] = GetTransparentColour();
            IDirectDrawSurface_SetColorKey(DAT_0079861c, 8, (LPDDCOLORKEY)off);
        }
    }
    DAT_00798608 = DAT_00668108;
    DAT_00798598 = DAT_0066809c;
    memset(&desc2, 0, sizeof(desc2));
    desc2.dwSize = 0x6c;
    if (IDirectDrawSurface_Lock(DAT_0079861c, NULL, &desc2, 0x21, NULL) == 0) {
        clip.left = 0;
        clip.right = desc2.dwWidth;
        clip.top = 0;
        clip.bottom = desc2.dwHeight;
        IntersectRect(&DAT_00668108, &clip, &SPRITE_ClipRect);
        off[0] = 0;
        off[1] = 0;
        DAT_007fea44 = GetTransparentColour();
        DAT_0066809c.lpSurface = desc2.lpSurface;
        DAT_0066809c.dwWidth = (short)sprite->width;
        DAT_0066809c.dwHeight = (short)sprite->height;
        DAT_0066809c.lPitch = desc2.lPitch;
        SoftPrint_Clear();
        r2.left = 0;
        r2.right = (short)sprite->width;
        r2.top = 0;
        r2.bottom = (short)sprite->height;
        if ((short)sprite->src_x < 0 || (short)sprite->src_y < 0) {
            // STRING: LEGOLAND 0x004bdd74
            printf("break");
        }
        FUN_00464ee0(sprite, &r2, off);
        IDirectDrawSurface_Unlock(DAT_0079861c, desc2.lpSurface);
    }
    IDirectDrawSurface_SetClipper(renderEngine, DAT_00668080);
    hr = IDirectDrawSurface_Blt(renderEngine, &dst, DAT_0079861c, &src, 0x1008000, NULL);
    for (;;) {
        if (hr != 0) {
            if (hr != 0x887601c2) {
                break;
            }
            if (IDirectDrawSurface_Restore(DAT_0079861c) != 0) {
                IDirectDrawSurface_SetClipper(renderEngine, NULL);
                DAT_0066809c = DAT_00798598;
                DAT_00668108 = DAT_00798608;
                return 0;
            }
            MakeSprite(sprite);
            if (IDirectDrawSurface_IsLost(DAT_00668070) == 0x887601c2) {
                if (IDirectDrawSurface_Restore(DAT_00668070) != 0) {
                    break;
                }
            }
            if (IDirectDrawSurface_Blt(renderEngine, &dst, DAT_0079861c, &src, 0x8000, NULL) != 0) {
                break;
            }
        }
        IDirectDrawSurface_SetClipper(renderEngine, NULL);
        DAT_0066809c = DAT_00798598;
        DAT_00668108 = DAT_00798608;
        return 1;
    }
    IDirectDrawSurface_SetClipper(renderEngine, NULL);
    DAT_0066809c = DAT_00798598;
    DAT_00668108 = DAT_00798608;
    return 0;
}

// FUNCTION: LEGOLAND 0x00489080
LEGO_EXPORT unsigned int RenderScaledSprite(struct Sprite *param_1, int param_2, int param_3, int param_4, int param_5) {
    int local[2];

    local[0] = 0;
    local[1] = 0;
    FUN_00488c80(param_1, param_2, param_3, param_4, param_5, local);
    return 1;
}

// FUNCTION: LEGOLAND 0x004890c0
LEGO_EXPORT unsigned int RenderBlock(int x, int y, int w, int h, unsigned int color) {
    RECT dst;
    DDBLTFX fx;

    dst.left = x;
    dst.top = y;
    dst.right = x + w;
    dst.bottom = y + h;
    fx.dwSize = 100;
    fx.dwFillColor = color;
    if (IntersectRect(&dst, &dst, &SPRITE_ClipRect) == 0) {
        return 1;
    }
    PushRenderingStatusAndUnlockVideoSurface();
    ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->SetClipper((LPDIRECTDRAWSURFACE)renderEngine, (LPDIRECTDRAWCLIPPER)DAT_00668080);
    if (((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->Blt((LPDIRECTDRAWSURFACE)renderEngine, &dst, NULL, NULL, 0x1000400, &fx) == 0) {
        ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->SetClipper((LPDIRECTDRAWSURFACE)renderEngine, NULL);
        PopRenderingStatus();
        return 1;
    }
    ((LPDIRECTDRAWSURFACE)renderEngine)->lpVtbl->SetClipper((LPDIRECTDRAWSURFACE)renderEngine, NULL);
    PopRenderingStatus();
    return 0;
}

/* Lock info filled by GetSprite (NULL locks the screen) and released by ReleaseSprite. */
struct SpriteLock {
    int pitch;
    int width;
    int height;
    unsigned char *bits;
    void *surface;
    int depth;
};

/* Draws `sprite` at (x, y) blended 50% over the screen, 16-bit modes only: each pair of 5:6:5 pixels is
 * averaged with the mask 0xf7def7de, transparent (zero) pairs are skipped, and every second source row is
 * written to two screen rows. As in the original, the blend reads the next screen pair, not the one it
 * writes. The original does the clipping and blending in an inline __asm block (it even reuses ebp as a
 * loop register); this is the C equivalent: same effect, but it cannot byte-match without __asm. */
// FUNCTION: LEGOLAND 0x00489190
LEGO_EXPORT int RenderTransSprite(struct Sprite *sprite, int x, int y) {
    struct SpriteLock screen;
    struct SpriteLock image;
    RECT dst;
    RECT off;
    unsigned short w;
    unsigned short h;
    int last;
    int rows;
    int cols;
    int c;
    unsigned char *d;
    unsigned char *s;
    unsigned int *dp;
    unsigned int *sp;
    unsigned int px;
    int result;

    w = sprite->width;
    h = sprite->height;
    if (!GetSprite((unsigned int *)&screen, NULL)) {
        return 0;
    }
    if (!GetSprite((unsigned int *)&image, sprite)) {
        return 0;
    }
    switch (DAT_00668088) {
    case 0:
        result = 0;
        break;
    case 1:
        result = 0;
        break;
    case 2:
        if (x < SPRITE_ClipRect.left) {
            dst.left = SPRITE_ClipRect.left;
            off.left = SPRITE_ClipRect.left - x;
        } else {
            dst.left = x;
            off.left = 0;
        }
        last = x + (w - 1);
        if (last > SPRITE_ClipRect.right) {
            dst.right = SPRITE_ClipRect.right;
            off.right = (w - 1) - (last - SPRITE_ClipRect.right);
        } else {
            dst.right = last;
            off.right = w - 1;
        }
        if (y < SPRITE_ClipRect.top) {
            dst.top = SPRITE_ClipRect.top;
            off.top = SPRITE_ClipRect.top - y;
        } else {
            dst.top = y;
            off.top = 0;
        }
        last = y + (h - 1);
        if (last > SPRITE_ClipRect.bottom) {
            dst.bottom = SPRITE_ClipRect.bottom;
            off.bottom = (h - 1) - (last - SPRITE_ClipRect.bottom);
        } else {
            dst.bottom = last;
            off.bottom = h - 1;
        }
        result = 0;
        if (dst.left < dst.right && dst.top < dst.bottom) {
            d = screen.bits + screen.pitch * dst.top + dst.left * 2;
            s = (unsigned char *)((unsigned int)(image.bits + image.pitch * off.top + off.left * 2) & ~3u);
            rows = off.bottom - off.top - 1;
            cols = off.right - off.left - 2;
            do {
                dp = (unsigned int *)d;
                sp = (unsigned int *)s;
                c = cols;
                do {
                    px = *sp & 0xf7def7de;
                    dp++;
                    if (px != 0) {
                        px = (px >> 1) + ((*dp & 0xf7def7de) >> 1);
                        dp[-1] = px;
                        *(unsigned int *)((unsigned char *)dp + screen.pitch - 4) = px;
                    }
                    sp++;
                    c -= 2;
                } while (c >= 0);
                s += image.pitch * 2;
                d += screen.pitch * 2;
                rows -= 2;
            } while (rows >= 0);
            result = (int)d;
        }
        break;
    }
    ReleaseSprite((struct Sprite *)&image);
    ReleaseSprite((struct Sprite *)&screen);
    return result;
}

// FUNCTION: LEGOLAND 0x00489390
LEGO_EXPORT void RenderThickBox(int x, int y, int w, int h, int thickness, unsigned int color) {
    int inner;
    RenderBlock(x, y, w, thickness, color);
    inner = h + thickness * -2;
    RenderBlock(x, y + thickness, thickness, inner, color);
    RenderBlock((x - thickness) + w, y + thickness, thickness, inner, color);
    RenderBlock(x, (y - thickness) + h, w, thickness, color);
}

// FUNCTION: LEGOLAND 0x00489410
LEGO_EXPORT void RenderBox(unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {
    RenderThickBox(a1, a2, a3, a4, 1, a5);
}
