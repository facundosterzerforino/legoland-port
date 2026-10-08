#include "render3d.h"
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"
#include "llidb.h"
#include "man3d.h"
#include "math.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render.h"
#include "resource.h"
#include "tilemap.h"

#include "../../port/port_asm.h"
#include "image_sprite.h"

// FUNCTION: LEGOLAND 0x00441800
LEGO_EXPORT void Render_SetViewport(struct tagRECT *viewport) {
    ViewportLeft = viewport->left;
    ViewportRight = viewport->right;
    ViewportTop = viewport->top;
    ViewportBottom = viewport->bottom;
}

struct RenderListNode {
    struct RenderListNode *next;
    unsigned char pad_4[8];
    short id;
};

// FUNCTION: LEGOLAND 0x00441830
struct RenderListNode *FUN_00441830(void *param_1, short *param_2) {
    struct RenderListNode *node = (struct RenderListNode *)DAT_0081c8cc;
    while (node != NULL) {
        if (memcmp(&node->id, param_2, sizeof(node->id)) == 0) {
            DAT_0081c8cc = node;
            return node;
        }
        node = node->next;
    }
    DAT_0081c8cc = NULL;
    return NULL;
}

struct ViewportEntry {
    unsigned char pad_0[0x2e];
    short seats;
    unsigned char pad_30[0xcc - 0x30];
    void *riders;
};

// FUNCTION: LEGOLAND 0x00441870
struct RenderListNode *FUN_00441870(struct ViewportEntry *param_1, short *param_2) {
    DAT_0081c8cc = param_1->riders;
    return FUN_00441830(param_1, param_2);
}

// FUNCTION: LEGOLAND 0x00441890
struct RenderListNode *FUN_00441890(void *param_1, short *param_2) {
    struct RenderListNode *node = (struct RenderListNode *)DAT_0081c8cc;
    if (node != NULL) {
        DAT_0081c8cc = node->next;
        return FUN_00441830(param_1, param_2);
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x004418c0
struct RenderListNode *FUN_004418c0(int param_1, struct ViewportEntry *param_2, short *param_3) {
    struct RenderListNode *node = FUN_00441870(param_2, param_3);
    int i = 0;
    if (node != NULL) {
        for (; i < param_2->seats; i++) {
            if (i == param_1) {
                return node;
            }
            node = FUN_00441890(param_2, param_3);
        }
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00441910
void RidePointToScreen(int *param_1, float *param_2, int *param_3) {
    float v[3];
    v[0] = param_2[0];
    v[1] = param_2[1];
    v[0] -= DAT_004ab4c0;
    v[1] -= DAT_004ab4bc;
    param_3[0] = (int)v[0] / 2;
    param_3[1] = (int)v[1] / 2;
    param_3[0] += param_1[7];
    param_3[1] += param_1[8];
}

// FUNCTION: LEGOLAND 0x00441980
void Put3DBlokeOnRide(int *param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    /* Port [library:asm]: the original is inline asm (fistp). Poses person param_4 from frame param_3 of track param_2 of
     * animation param_1: the position from RidePointToScreen, offset by (param_5, param_6), and the 16.16 fixed-point
     * orientation from the frame's 3x3 float matrix, with axes 1 and 2 swapped and some signs flipped. */
    static const int src_col[3] = {0, 2, 1};
    static const int row_sign[3] = {1, -1, -1};
    static const int col_sign[3] = {-1, 1, 1};
    struct Person *person = (struct Person *)param_4;
    float *frame_data;
    int pos[2];
    int frame = param_3;
    int i;
    int j;

    if (frame < 0) {
        frame = 0;
    }
    if (frame >= param_1[0]) {
        frame = param_1[0] - 1;
    }
    /* each frame is 12 floats: the position, then the 3x3 matrix */
    frame_data = (float *)((char *)((int **)param_1[9])[param_2] + frame * 48);
    RidePointToScreen(param_1, frame_data, pos);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            person->m[j * 3 + i] = row_sign[j] * col_sign[i] * PortRound(frame_data[3 + i * 3 + src_col[j]] * 65536.0f);
        }
    }
    SetPersonPosition(person, pos[0] + param_5, pos[1] + param_6);
}

struct BlokeRideInner {
    unsigned char pad_0[0x62];
    unsigned char flags;
};

struct BlokeRideNode {
    unsigned char pad_0[8];
    struct BlokeRideInner *inner;
    unsigned char pad_c[4];
    unsigned int field_10;
};

// FUNCTION: LEGOLAND 0x00441a60
LEGO_EXPORT void Put3DBlokesOnRide(struct ViewportEntry *param_1, unsigned char *param_2, int param_3, int *param_4) {
    struct Point coords;
    int i;

    coords = GetScreenCoordsForObject(param_2, param_1);
    i = 0;
    if (param_4[1] > 0) {
        do {
            struct BlokeRideNode *node = (struct BlokeRideNode *)FUN_004418c0(i, param_1, (short *)param_2);
            if (node != NULL && (node->inner->flags & 0x80) != 0) {
                Put3DBlokeOnRide(param_4, i, param_3, node->field_10, coords.x, coords.y);
            }
            i = i + 1;
        } while (i < param_4[1]);
    }
}

// FUNCTION: LEGOLAND 0x00441ad0
LEGO_EXPORT void Put3DBlokesOnRide2(Element *ride, Element *obj) {
    struct BlokeRideNode *node = (struct BlokeRideNode *)FUN_00441870((struct ViewportEntry *)ride, (short *)obj);
    if (node != NULL) {
        while ((node->inner->flags & 0x80) != 0) {
            node = (struct BlokeRideNode *)FUN_00441890(ride, (short *)obj);
            if (node == NULL) {
                return;
            }
        }
        while (node != NULL) {
            if (node->field_10 != 0) {
                FUN_004401b0((struct Person *)node->field_10, (struct Bloke *)node->inner);
            }
            node = (struct BlokeRideNode *)FUN_00441890(ride, (short *)obj);
            if (node == NULL) {
                return;
            }
            while ((node->inner->flags & 0x80) != 0) {
                node = (struct BlokeRideNode *)FUN_00441890(ride, (short *)obj);
                if (node == NULL) {
                    return;
                }
            }
        }
    }
}

struct RenderNode {
    struct RenderNode *next;
    unsigned char pad_4[4];
    void *data_ptr;
    unsigned short id;
};

struct RenderBloke {
    unsigned short id;
};

struct RenderData {
    unsigned char pad_0[0x62];
    unsigned char flags;
};

struct RenderContext {
    unsigned char pad_0[0xcc];
    struct RenderNode *bloke_list;
};

// FUNCTION: LEGOLAND 0x00441b60
LEGO_EXPORT void RenderBlokesNotInSeats(unsigned int a1, unsigned int a2) {
    struct RenderContext *context = (struct RenderContext *)a1;
    struct RenderBloke *bloke = (struct RenderBloke *)a2;
    struct RenderNode *node = context->bloke_list;

    while (node != NULL) {
        if (bloke->id == node->id) {
            struct RenderData *data = (struct RenderData *)node->data_ptr;
            if ((data->flags & 0x80) == 0) {
                IP_RenderBlokeIn3DNow((struct Bloke *)data);
            }
        }
        node = node->next;
    }
}

struct RinData {
    unsigned char pad_0[0x10];
    int sprite_count;
    int data_count;
    struct Sprite **sprites;
    void **datas;
};

#pragma intrinsic(memset)

// FUNCTION: LEGOLAND 0x00441ba0
LEGO_EXPORT struct RinData *LoadRin(const char *path, const char *dir) {
    struct RinData *rin;
    struct ResFile *file;
    char name[264];
    char sprite_name[264];
    char c;
    int i;
    int j;

    rin = (struct RinData *)malloc(0x20);
    file = RES_OpenFile(path);
    if (file != NULL) {
        memset(rin, 0, 0x20);
        RES_ReadFile(file, &rin->sprite_count, 4);
        rin->sprites = (struct Sprite **)malloc(rin->sprite_count << 2);
        i = 0;
        if (rin->sprite_count > 0) {
            do {
                char *p = name;
                do {
                    RES_ReadFile(file, &c, 1);
                    *p = c;
                    p++;
                } while (c != '\0');
                // STRING: LEGOLAND 0x004b7cf4
                sprintf(sprite_name, "%s\\%s.lls", dir, name);
                rin->sprites[i] = LoadSprite(sprite_name, 1);
                i++;
            } while (i < rin->sprite_count);
        }
        RES_ReadFile(file, &rin->data_count, 4);
        rin->datas = (void **)malloc(rin->data_count << 2);
        if (rin->datas != NULL && (j = 0, rin->data_count > 0)) {
            do {
                rin->datas[j] = malloc(rin->sprite_count << 2);
                RES_ReadFile(file, rin->datas[j], rin->sprite_count << 2);
                j++;
            } while (j < rin->data_count);
        }
        RES_CloseFile(file);
    }
    return rin;
}

#pragma function(memset)

// FUNCTION: LEGOLAND 0x00441cf0
LEGO_EXPORT void UnLoadRin(struct RinData *rin) {
    int i;
    int count;

    count = rin->sprite_count;
    for (i = 0; i < count; i++) {
        KillSprite(rin->sprites[i]);
        count = rin->sprite_count;
    }
    free(rin->sprites);

    count = rin->data_count;
    for (i = 0; i < count; i++) {
        free(rin->datas[i]);
        count = rin->data_count;
    }
    free(rin->datas);

    free(rin);
}

struct RinRender {
    int x;
    int y;
    int *remap_table;
    int *data_table;
    int loop_count;
    int modulo;
    int **index_array;
    int **frame_array;
};

// FUNCTION: LEGOLAND 0x00441d60
LEGO_EXPORT void RenderUsingRin(struct RinRender *param_1, int param_2, struct ViewportEntry *param_3, unsigned char *param_4) {
    struct Point coords;
    int idx;
    void *base;
    int i;
    struct Point offset;

    coords = GetScreenCoordsForObject(param_4, param_3);
    idx = param_2;
    if (param_2 >= param_1->modulo) {
        idx = param_2 % param_1->modulo;
    }
    base = param_1->frame_array[idx];
    for (i = param_1->loop_count - 1; i >= 0; i--) {
        int sprite_id = ((int *)base)[i];
        struct BlokeRideNode *node;
        int *frame;
        if (param_1->data_table == NULL) {
            node = (struct BlokeRideNode *)FUN_004418c0(sprite_id, param_3, (short *)param_4);
        } else {
            node = (struct BlokeRideNode *)FUN_004418c0(param_1->data_table[param_1->remap_table[sprite_id]], param_3, (short *)param_4);
        }
        if (node != NULL && (node->inner->flags & 0x80) != 0) {
            IP_RenderBlokeIn3DNow((struct Bloke *)node->inner);
        }
        frame = (int *)param_1->index_array[sprite_id];
        if (frame != NULL) {
            LLSSetFrame((struct LLS *)GetLLSForSprite((struct SpriteLLS *)frame), param_2);
        }
        offset.x = param_1->x;
        offset.y = param_1->y;
        AdjustOffsetForViewMode(&offset);
        frame = (int *)param_1->index_array[sprite_id];
        if (frame != NULL) {
            PrintSprite((struct Sprite *)frame, coords.x + offset.x, coords.y + offset.y, 0, 0);
        }
    }
}

struct SpriteLLS {
    unsigned char pad_0[8];
    unsigned int *field_8;
};

// FUNCTION: LEGOLAND 0x00441e80
LEGO_EXPORT unsigned int GetLLSForSprite(struct SpriteLLS *sprite) {
    if (sprite != NULL) {
        return *sprite->field_8;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00441ea0
LEGO_EXPORT struct LLS *GetLLSForLayer(struct Sprite *sprite, unsigned int index) {
    struct Sprite *layer = sprite->group->subs[index];
    if (layer == NULL) {
        return NULL;
    }
    return *layer->lls;
}

// FUNCTION: LEGOLAND 0x00441ec0
LEGO_EXPORT struct Sprite *GetSpriteForLayer(struct Sprite *sprite, unsigned int index) {
    return sprite->group->subs[index];
}

// FUNCTION: LEGOLAND 0x00441ee0
LEGO_EXPORT struct Point GetRenderOffsetForLayer(struct Sprite *sprite, int index) {
    struct SpriteGroup *group = sprite->group;
    struct Point r;
    r.x = group->xoffs[index];
    r.y = group->yoffs[index];
    return r;
}

// FUNCTION: LEGOLAND 0x00441f00
LEGO_EXPORT void StopLayerPlaying(struct Sprite *sprite, unsigned int index) {
    LLSStop((unsigned int)GetLLSForLayer(sprite, index));
}

#pragma intrinsic(memset)

// FUNCTION: LEGOLAND 0x00441f20
LEGO_EXPORT unsigned short *LoadPalette(unsigned int path) {
    unsigned short *palette;
    unsigned short *out;
    struct ResFile *file;
    int i;
    unsigned char g;
    unsigned char b;
    unsigned char header[8];

    palette = (unsigned short *)malloc(0x200);
    if (palette != NULL) {
        memset(palette, 0, 0x200);
        file = RES_OpenFile((const char *)path);
        if (file != NULL) {
            RES_ReadFile(file, header, 8);
            out = palette;
            for (i = 0x100; i != 0; i--) {
                RES_ReadFile(file, &path, 1);
                RES_ReadFile(file, &g, 1);
                RES_ReadFile(file, &b, 1);
                if (DisplayPixelFormat == 2) {
                    *out = (((path & 0xf8) << 5 | (g & 0xfc)) << 3) | (b >> 3);
                } else {
                    *out = (unsigned short)((((((unsigned char)path & 0xf8) << 5) | (g & 0xf8)) << 2) | (b >> 3));
                }
                out++;
            }
            RES_CloseFile(file);
        }
    }
    return palette;
}

#pragma function(memset)

struct CellContainer {
    unsigned char pad_0[4];
    int field_4;
    unsigned char pad_8[0x30 - 8];
    short *entries;
};

struct CellEntry {
    short field_0;
    unsigned char x;
    unsigned char y;
    unsigned char field_4;
    unsigned char field_5;
};

// FUNCTION: LEGOLAND 0x00442040
void RemapTexCoordsToCell(struct CellContainer *param_1, int param_2, int param_3, float *param_4, int param_5) {
    struct CellEntry *entry1 = (struct CellEntry *)((char *)param_1->entries + param_2 * 6);
    struct CellEntry *entry2 = (struct CellEntry *)((char *)param_1->entries + param_3 * 6);
    unsigned char bVar3 = entry1->field_4;
    int iVar22 = entry2->field_0 + param_1->field_4;
    int lo_x = entry1->x;
    int hi_x = bVar3 + entry1->x + 1;
    int lo_y = entry1->y;
    int hi_y = entry1->field_5 + entry1->y + 1;
    int idx1 = entry1->field_0 + param_1->field_4;
    float *p;
    int local_18;

    if (param_5 > 0) {
        local_18 = param_5;
        p = param_4 + 3;
        do {
            if ((*(unsigned int *)(p - 3) & 0x2000) == 0 && *(int *)(p - 1) == idx1) {
                float v0 = p[0];
                float v1 = p[1];
                float v2 = p[2];
                float v3 = p[3];
                float v4 = p[4];
                float v5 = p[5];
                float s0, s1;
                if (v0 < FLOAT_004ab390) v0 = 0.0f;
                if (1.0 < v0) v0 = 1.0f;
                if (v1 < FLOAT_004ab390) v1 = 0.0f;
                if (1.0 < v1) v1 = 1.0f;
                if (v2 < FLOAT_004ab390) v2 = 0.0f;
                if (1.0 < v2) v2 = 1.0f;
                if (v3 < FLOAT_004ab390) v3 = 0.0f;
                if (1.0 < v3) v3 = 1.0f;
                if (v4 < FLOAT_004ab390) v4 = 0.0f;
                if (1.0 < v4) v4 = 1.0f;
                if (v5 < FLOAT_004ab390) v5 = FLOAT_004ab390;
                if (1.0 < v5) v5 = 1.0f;
                s0 = (float)DAT_0081c0c0[idx1 * 2];
                v0 = s0 * v0;
                v2 = s0 * v2;
                v4 = s0 * v4;
                s1 = (float)DAT_0081c0c0[idx1 * 2 + 1];
                v1 = s1 * v1;
                v3 = s1 * v3;
                v5 = s1 * v5;
                if ((float)lo_x <= v0 && v0 <= (float)hi_x &&
                    (float)lo_x <= v2 && v2 <= (float)hi_x &&
                    (float)lo_x <= v4 && v4 <= (float)hi_x &&
                    (float)lo_y <= v1 && v1 <= (float)hi_y &&
                    (float)lo_y <= v3 && v3 <= (float)hi_y &&
                    (float)lo_y <= v5 && v5 <= (float)hi_y) {
                    float e1_2 = (float)entry1->x;
                    float e1_4 = (float)entry1->field_4;
                    float e2_4 = (float)entry2->field_4;
                    float e2_2 = (float)entry2->x;
                    float t0 = (float)DAT_0081c0c0[iVar22 * 2];
                    float e1_3 = (float)entry1->y;
                    float e1_5 = (float)entry1->field_5;
                    float e2_5 = (float)entry2->field_5;
                    float e2_3 = (float)entry2->y;
                    float t1 = (float)DAT_0081c0c0[iVar22 * 2 + 1];
                    *(int *)(p - 1) = entry2->field_0 + param_1->field_4;
                    p[1] = (((v1 - e1_3) / e1_5) * e2_5 + e2_3) / t1;
                    p[2] = (((v2 - e1_2) / e1_4) * e2_4 + e2_2) / t0;
                    p[3] = (((v3 - e1_3) / e1_5) * e2_5 + e2_3) / t1;
                    p[4] = (((v4 - e1_2) / e1_4) * e2_4 + e2_2) / t0;
                    p[0] = (((v0 - e1_2) / e1_4) * e2_4 + e2_2) / t0;
                    p[5] = (((v5 - e1_3) / e1_5) * e2_5 + e2_3) / t1;
                }
            }
            p = p + 9;
            local_18 = local_18 - 1;
        } while (local_18 != 0);
    }
}

struct ColourRef {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
};

struct TexEntry {
    unsigned int flags;
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char pad_7[1];
    unsigned int field_8;
    unsigned char pad_c[0x24 - 0xc];
};

// FUNCTION: LEGOLAND 0x004424e0
void FUN_004424e0(struct ColourRef *param_1, struct ColourRef *param_2, struct ColourRef *param_3, struct ColourRef *param_4, int param_5, int param_6) {
    int i = 0;
    int count;
    struct TexEntry *entry;

    if (param_6 > 0) {
        entry = (struct TexEntry *)param_5;
        count = param_6;
        do {
            if ((entry->flags & 0x2000) != 0 && entry->b2 == param_1->b2 &&
                entry->b1 == param_1->b1 && entry->b0 == param_1->b0) {
                if (i >= count >> 1) {
                    entry->b2 = param_2->b2;
                    entry->b1 = param_2->b1;
                    entry->b0 = param_2->b0;
                    param_5 = *(int *)param_2;
                } else {
                    entry->b2 = param_4->b2;
                    entry->b1 = param_4->b1;
                    entry->b0 = param_4->b0;
                    param_5 = *(int *)param_4;
                }
                entry->field_8 = FUN_00486280(0x40, &param_5);
                count = param_6;
            }
            entry = (struct TexEntry *)((char *)entry + 0x24);
            i = i + 1;
        } while (i < count);
    }
}

// FUNCTION: LEGOLAND 0x00442580
void *FUN_00442580(struct Person *person, void *context, unsigned int src, unsigned int count, unsigned int flag) {
    int *arrA;
    int idxI;
    int valC;
    int *arrD;
    int idxJ;
    int valE;
    struct ColourRef z0;
    struct ColourRef z1;
    struct ColourRef z2;
    struct ColourRef z3;
    int a;
    int b;
    void *mem;
    int size;
    int modI;
    int modJ;

    if (person->character == 1) {
        if (flag == 0) {
            arrA = DAT_00655a38;
            arrD = DAT_0062fea8;
            valC = DAT_00641000;
            modI = DAT_0063810c;
            modJ = DAT_0062feb8[0];
            valE = DAT_0064cd90;
            z0.b0 = z0.b1 = z0.b2 = 0x56;
            z3.b0 = z3.b1 = z3.b2 = 0;
        } else {
            arrA = DAT_0062fef8;
            arrD = DAT_0064cd8c;
            modI = DAT_0064cd88;
            modJ = DAT_0063835c;
            valC = DAT_00638110;
            valE = DAT_00638108;
            z0.b0 = z0.b1 = z0.b2 = 0x56;
            z3.b0 = z3.b1 = z3.b2 = 1;
        }
        if (person->field_80 == -1) {
            idxI = rand() % modI;
            person->field_80 = idxI;
        } else {
            idxI = person->field_80;
        }
        if (person->field_7c == -1) {
            idxJ = rand() % modJ;
            person->field_7c = idxJ;
        } else {
            idxJ = person->field_7c;
        }
        b = person->field_8c;
        if (b == -1) {
            do {
                b = rand() & 7;
            } while (b == 7);
            person->field_8c = b;
        }
        a = person->field_90;
        if (a == -1) {
            a = rand() & 7;
            person->field_90 = a;
        }
    } else {
        for (;;) {
            if (person->character == 3) {
                a = 4;
                b = 1;
            } else if (person->character == 2) {
                a = 0;
                b = 3;
            } else {
                break;
            }
            z0.b0 = z0.b1 = z0.b2 = 0x56;
            z3.b0 = z3.b1 = z3.b2 = 0;
            break;
        }
    }
    z1.b2 = BlokeColours[a * 3];
    z1.b1 = BlokeColours[a * 3 + 1];
    z1.b0 = BlokeColours[a * 3 + 2];
    z2.b2 = BlokeColours[b * 3];
    z2.b1 = BlokeColours[b * 3 + 1];
    z2.b0 = BlokeColours[b * 3 + 2];
    size = count * 36;
    mem = malloc(size);
    if (mem != 0) {
        memcpy(mem, (void *)src, size);
        if ((int)person->character < 2) {
            RemapTexCoordsToCell(context, valC, arrA[idxI], (float *)mem, count);
            RemapTexCoordsToCell(context, valE, arrD[idxJ], (float *)mem, count);
        }
        FUN_004424e0(&z0, &z1, &z3, &z2, (int)mem, count);
    }
    return mem;
}

// FUNCTION: LEGOLAND 0x004427e0
char *RES_ReadLine(struct ResFile *param_1, char *param_2, int param_3) {
    struct ResFile *file = param_1;
    char *c = (char *)&param_1;
    int count = 0;
    int read;
    char *dst = param_2;

    do {
        read = RES_ReadFile(file, &param_1, 1);
        if (read != 0) {
            if (*c == '\r') goto skip_lf;
            if (*c == '\n') goto terminate;
            *dst = *c;
            dst++;
            count++;
        }
        if (*c == '\r') goto skip_lf;
    } while (*c != '\n' && count < param_3 && read != 0);
    if (*c == '\r') {
    skip_lf:
        RES_ReadFile(file, &param_1, 1);
    }
terminate:
    *dst = '\0';
    if (read == 0 && count == 0) {
        return NULL;
    }
    return param_2;
}

// FUNCTION: LEGOLAND 0x00442860
int FindIndexInStringList(char *param_1, char *param_2) {
    int index = 0;

    while (_stricmp(param_1, param_2) != 0) {
        param_1 = param_1 + strlen(param_1) + 1;
        index = index + 1;
        if (strlen(param_1) == 0) {
            return -1;
        }
    }
    return index;
}

// FUNCTION: LEGOLAND 0x004428c0
unsigned char *GetNthStringInList(unsigned char *str, int count) {
    unsigned char *result = str;
    if (count > 0) {
        do {
            result = result + strlen((char *)result) + 1;
            count = count - 1;
        } while (count != 0);
    }
    return result;
}

/* A 3D data list (Load3DDataFile, e.g. visitor\altman.txt) is sections of "Name\0" + int count + strings
 * ending with an empty one. param_2 picks the section (0 faces, 1 chests), param_3 the string in it.
 * volatile: the original reloads param_3 from the stack at each use. values is only set when section 0 has
 * strings; the original keeps it in param_1's stack slot, and the game's files always have them. */
// FUNCTION: LEGOLAND 0x004428f0
unsigned char *Get3DDataListString(char *param_1, int param_2, volatile int param_3) {
    char *values;
    char *names;
    char *p;
    int flag;

    if (param_1 != NULL) {
        p = param_1 + strlen(param_1) + 1;
        flag = *(int *)p;
        p = p + 4;
        names = p;
        if (flag != 0) {
            do {
                p = p + strlen(p) + 1;
            } while (strlen(p) != 0);
            p++;
            values = p + strlen(p) + 1 + 4;
        }
    } else {
        names = (char *)param_3;
    }
    switch (param_2) {
    case 1:
        return GetNthStringInList((unsigned char *)values, param_3);
    }
    return GetNthStringInList((unsigned char *)names, param_3);
}

// FUNCTION: LEGOLAND 0x00442980
void FUN_00442980(const char *param_1, const char *param_2, const char *param_3, int param_4, unsigned int param_5) {
    int idx = 0;
    int count1;
    int count2;
    char *list_mid;
    char *list1;
    char *name;
    char *list2;
    int *out_b;
    char *data;
    int *out_a;
    int *arr2;
    int *arr1;
    int v1, v2, v3, v4, v5;
    char word[128];
    char line[512];
    char path[256];
    struct ResFile *file;
    char *p;
    char *q;
    char *dst;
    char c;
    int r;

    data = (char *)Load3DDataFile(param_3, param_1);
    if (data != NULL) {
        name = data;
        p = data + strlen(data) + 1;
        count1 = *(int *)p;
        list1 = p + 4;
        if (count1 != 0) {
            q = list1;
            do {
                q = q + strlen(q) + 1;
            } while (strlen(q) != 0);
            q = q + 1;
            list_mid = q;
            q = q + strlen(q) + 1;
            count2 = *(int *)q;
            list2 = q + 4;
        }
    } else {
        count1 = 0;
    }
    sprintf(path, ".\\3ddata\\new\\%s\\%s", param_3, param_2);
    if (param_4 == 0) {
        DAT_0063810c = count1;
        DAT_00655a38 = (int *)malloc(count1 * 4);
        DAT_0062feb8[0] = count2;
        DAT_0062fea8 = (int *)malloc(count2 * 4);
        arr1 = DAT_00655a38;
        arr2 = DAT_0062fea8;
        out_b = &DAT_0064cd90;
        out_a = &DAT_00641000;
    } else {
        DAT_0064cd88 = count1;
        DAT_0062fef8 = (int *)malloc(count1 * 4);
        DAT_0063835c = count2;
        DAT_0064cd8c = (int *)malloc(count2 * 4);
        arr1 = DAT_0062fef8;
        arr2 = DAT_0064cd8c;
        out_b = &DAT_00638108;
        out_a = &DAT_00638110;
    }
    file = RES_OpenFile(path);
    if (file != NULL) {
        RES_ReadLine(file, line, 512);
        RES_ReadLine(file, line, 512);
        if (RES_ReadLine(file, line, 512) != NULL) {
            do {
                p = line;
                dst = path;
                do {
                    c = (char)tolower(*p);
                    p++;
                    if (c == '.') c = 0;
                    *dst = c;
                    dst++;
                } while (c != 0);
                sscanf(p, "%s %i %i %i %i %i", word, &v5, &v4, &v3, &v2, &v1);
                if (strcmp(name, path) == 0) {
                    *out_a = idx;
                } else if (strcmp(list_mid, path) == 0) {
                    *out_b = idx;
                }
                if (count1 != 0) {
                    r = FindIndexInStringList(list1, path);
                    if (r != -1) arr1[r] = idx;
                }
                if (count2 != 0) {
                    r = FindIndexInStringList(list2, path);
                    if (r != -1) arr2[r] = idx;
                }
                idx++;
            } while (RES_ReadLine(file, line, 512) != NULL);
        }
    }
    RES_CloseFile(file);
    free(data);
}

// FUNCTION: LEGOLAND 0x00442c70
unsigned int FUN_00442c70(void) {
    if (DAT_00655a38) {
        free(DAT_00655a38);
    }
    if (DAT_0062fea8) {
        free(DAT_0062fea8);
    }
    if (DAT_0062fef8) {
        free(DAT_0062fef8);
    }
    if (DAT_0064cd8c) {
        free(DAT_0064cd8c);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00442cc0
LEGO_EXPORT struct Point GetScreenCoordsForObject(TileId *tile, struct Ride *ride) {
    int bounds[4];
    struct Point ref;
    int iVar1;
    int iVar2;
    struct Point r;

    ref.x = tile->pos.x;
    ref.y = tile->pos.y;
    GetTileBounds(&ref, bounds);
    iVar2 = ride->field_14;
    iVar1 = ride->field_18;
    if (iVar2 < 0) {
        iVar2 = -(-iVar2 >> 1);
    } else {
        iVar2 = iVar2 >> 1;
    }
    if (iVar1 < 0) {
        r.x = iVar2 + bounds[0];
        r.y = bounds[1] - (-iVar1 >> 1);
        return r;
    }
    r.x = iVar2 + bounds[0];
    r.y = bounds[1] + (iVar1 >> 1);
    return r;
}

// FUNCTION: LEGOLAND 0x00442d30
LEGO_EXPORT void AdjustOffsetForViewMode(struct Point *param_1) {
    int temp0;
    int temp4;

    temp0 = param_1->x;
    if (temp0 < 0) {
        temp0 = -((-temp0) >> 1);
    } else {
        temp0 = temp0 >> 1;
    }
    param_1->x = temp0;

    temp4 = param_1->y;
    if (temp4 < 0) {
        temp4 = -((-temp4) >> 1);
    } else {
        temp4 = temp4 >> 1;
    }
    param_1->y = temp4;
}

// FUNCTION: LEGOLAND 0x00442d60
LEGO_EXPORT void AdjustBlokePosition(struct Point *pos) {
    pos->x -= 0x4b;
    pos->y -= 0x4d;
}

// FUNCTION: LEGOLAND 0x00442d80
LEGO_EXPORT void UnAdjustBlokePosition(struct Point *pos) {
    pos->x += 0x4b;
    pos->y += 0x4d;
}

struct Vec3 {
    float x;
    float y;
    float z;
};

// FUNCTION: LEGOLAND 0x00442da0
void CrossProduct(struct Vec3 *a, struct Vec3 *b, struct Vec3 *out) {
    out->x = b->z * a->y - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
}

// FUNCTION: LEGOLAND 0x00442de0
float DotProduct(struct Vec3 *param_1, struct Vec3 *param_2) {
    return param_1->z * param_2->z + param_1->y * param_2->y + param_1->x * param_2->x;
}

// FUNCTION: LEGOLAND 0x00442e00
LEGO_EXPORT int TMNegParity(float *param_1) {
    struct Vec3 a;
    struct Vec3 b;
    struct Vec3 cross;

    a.x = param_1[0];
    a.y = param_1[1];
    a.z = param_1[2];
    b.x = param_1[3];
    b.y = param_1[4];
    b.z = param_1[5];
    CrossProduct(&a, &b, &cross);
    a.x = param_1[6];
    a.y = param_1[7];
    a.z = param_1[8];
    /* [port] the original compares on the x87 (fcomp; test ah, 1), where an unordered result (NaN) also counts
     * as "less", so NaN returns 1. FUN_00440a30 passes the 16.16 fixed-point orientation read as floats, and
     * its m[4] (-0x10000 = 0xffff0000) is a NaN, so there the original always gets 1. An SSE compare would say
     * 0 and mirror every 3D person's triangles, so the back faces were drawn instead of the front ones. */
    if (!(DotProduct(&cross, &a) >= DAT_004ab4c8)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00442e90
LEGO_EXPORT void RenderItems_New(void) {
    DAT_0062feec[0] = (unsigned int)&DAT_00630108;
    RenderItemCount = 0;
}

// FUNCTION: LEGOLAND 0x00442eb0
LEGO_EXPORT void RenderItem_Link(struct RenderItemNode **head, struct RenderItemNode *node, int key) {
    struct RenderItemNode *cur;

    node->next = NULL;
    node->prev = NULL;
    cur = *head;
    if (cur == NULL) {
        *head = node;
        return;
    }
    for (;;) {
        if (key <= cur->key) {
            node->next = cur;
            node->prev = cur->prev;
            if (cur->prev != NULL) {
                cur->prev->next = node;
            }
            cur->prev = node;
            if (cur == *head) {
                *head = node;
            }
            return;
        }
        if (cur->next == NULL) {
            cur->next = node;
            node->prev = cur;
            return;
        }
        cur = cur->next;
    }
}

struct RenderItem {
    unsigned int field_0;
    unsigned int field_4;
};

struct BlokeRenderSrc {
    unsigned char pad_0[8];
    unsigned int field_8;
};

// FUNCTION: LEGOLAND 0x00442f20
LEGO_EXPORT void AddBlokeToRenderList(struct RenderItemNode **head, struct BlokeRenderSrc *src, int key) {
    struct RenderItem *item = (struct RenderItem *)AllocRenderItem();
    item->field_4 = src->field_8;
    item->field_0 = key;
    RenderItem_Link(head, (struct RenderItemNode *)item, key);
}

// FUNCTION: LEGOLAND 0x00442f50
unsigned int AllocRenderItem(void) {
    unsigned int result = DAT_0062feec[0];
    DAT_0062feec[0] = DAT_0062feec[0] + 16;
    RenderItemCount = RenderItemCount + 1;
    return result;
}

struct BlokeListNode {
    unsigned int field_0;
    unsigned int field_4;
    struct BlokeListNode *next;
};

struct BlokeListHead {
    struct BlokeListNode *next;
};

// FUNCTION: LEGOLAND 0x00442f70
LEGO_EXPORT void RenderBlokeList(struct BlokeListHead *list) {
    struct BlokeListNode *node = list->next;
    while (node) {
        if (node->field_4) {
            IP_RenderBlokeIn3DNow((struct Bloke *)node->field_4);
        }
        node = node->next;
    }
}

struct MapCellElement {
    struct RenderObjectVtable3 *vtable;
};

struct RenderObjectVtable3 {
    unsigned char pad_0[0xc];
    struct ObjClassNode *class_node;
};

struct RideInstance {
    unsigned char pad_0[0xc];
    unsigned short flags;
};

// FUNCTION: LEGOLAND 0x00442fa0
LEGO_EXPORT void Ride_SetFlagToNotLetAnyoneOn(void *param_1) {
    struct MapCellElement *cell;
    struct RideInstance *instance;
    unsigned char *coords = param_1;
    int x = coords[0];
    int y = coords[1];

    if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
        cell = (struct MapCellElement *)((char *)GameMap[y] + x * 0x14);
    } else {
        cell = 0;
    }
    instance = (struct RideInstance *)GetInstanceOfClass((struct Ride *)cell->vtable->class_node, (const TileId *)param_1);
    if (instance != 0) {
        *(unsigned char *)&instance->flags |= 2;
    }
}

// FUNCTION: LEGOLAND 0x00443000
LEGO_EXPORT void Ride_ClearFlagToNotLetAnyoneOn(void *param_1) {
    struct MapCellElement *cell;
    struct RideInstance *instance;
    unsigned char *coords = param_1;
    int x = coords[0];
    int y = coords[1];

    if (x >= 0 && x < (int)lpConfig->width && y >= 0 && y < (int)lpConfig->height) {
        cell = (struct MapCellElement *)((char *)GameMap[y] + x * 0x14);
    } else {
        cell = 0;
    }
    instance = (struct RideInstance *)GetInstanceOfClass((struct Ride *)cell->vtable->class_node, (const TileId *)param_1);
    if (instance != 0) {
        instance->flags &= 0xfffd;
    }
}

// FUNCTION: LEGOLAND 0x00443060
LEGO_EXPORT void RenderItems2_New(void) {
    DAT_0062fef0 = (unsigned int)&DAT_00638218;
    RenderItem2Count = 0;
}

// FUNCTION: LEGOLAND 0x00443080
LEGO_EXPORT void RenderItem2_Link(struct RenderItemNode **head, struct RenderItemNode *node, int key) {
    struct RenderItemNode *cur;

    node->next = NULL;
    node->prev = NULL;
    cur = *head;
    if (cur == NULL) {
        *head = node;
        return;
    }
    for (;;) {
        if (key <= cur->key) {
            node->next = cur;
            node->prev = cur->prev;
            if (cur->prev != NULL) {
                cur->prev->next = node;
            }
            cur->prev = node;
            if (cur == *head) {
                *head = node;
            }
            return;
        }
        if (cur->next == NULL) {
            cur->next = node;
            node->prev = cur;
            return;
        }
        cur = cur->next;
    }
}

struct RenderItem2 {
    unsigned int field_0;
    unsigned int field_4;
};

// FUNCTION: LEGOLAND 0x004430f0
LEGO_EXPORT void RenderItem2_AddItem(struct RenderItemNode **head, unsigned int param_2, int key) {
    struct RenderItem2 *item = (struct RenderItem2 *)FUN_00443120();
    item->field_4 = param_2;
    item->field_0 = key;
    RenderItem2_Link(head, (struct RenderItemNode *)item, key);
}

// FUNCTION: LEGOLAND 0x00443120
unsigned int FUN_00443120(void) {
    unsigned int result = DAT_0062fef0;
    DAT_0062fef0 = DAT_0062fef0 + 16;
    RenderItem2Count = RenderItem2Count + 1;
    return result;
}

struct BlokeSex0 {
    unsigned char pad_0[4];
    struct BlokeSex1 *field_4;
};

struct BlokeSex1 {
    unsigned char pad_0[0x80];
    unsigned int field_80;
    unsigned int field_84;
    unsigned char pad_88[0x8c - 0x88];
    unsigned int field_8c;
    unsigned int field_90;
};

// FUNCTION: LEGOLAND 0x00443140
LEGO_EXPORT unsigned int GetSexOfBloke(struct BlokeSex0 *param_1) {
    struct BlokeSex1 *inner = param_1->field_4;
    return inner->field_84;
}

// FUNCTION: LEGOLAND 0x00443150
LEGO_EXPORT char *GetFaceTextureNameOfBloke(struct BlokeSex0 *param_1) {
    struct BlokeSex1 *inner = param_1->field_4;
    void *ptr;
    char *name;
    switch (inner->field_84) {
    case 0:
        ptr = AltManFileData;
        break;
    case 1:
        ptr = AltWomanFileData;
        break;
    }
    name = (char *)Get3DDataListString((char *)ptr, 0, inner->field_80);
    // STRING: LEGOLAND 0x004b7d24
    _stricmp(name, "chest girly1");
    // STRING: LEGOLAND 0x004b7d14
    return "chest girly2";
}

// FUNCTION: LEGOLAND 0x004431a0
LEGO_EXPORT char *GetChestTextureNameOfBloke(struct BlokeSex0 *param_1) {
    struct BlokeSex1 *inner = param_1->field_4;
    void *ptr;
    char *name;
    switch (inner->field_84) {
    case 0:
        ptr = AltManFileData;
        break;
    case 1:
        ptr = AltWomanFileData;
        break;
    }
    name = (char *)Get3DDataListString((char *)ptr, 1, inner->field_80);
    _stricmp(name, "chest girly1");
    return "chest girly2";
}

// FUNCTION: LEGOLAND 0x004431f0
LEGO_EXPORT unsigned int GetLegColourOfBloke(struct BlokeSex0 *param_1) {
    unsigned int idx = param_1->field_4->field_8c;
    unsigned int r = BlokeColours[idx * 3];
    unsigned int b = BlokeColours[idx * 3 + 2];
    unsigned int g = BlokeColours[idx * 3 + 1];
    return (((r << 8) | g) << 8) | b;
}

// FUNCTION: LEGOLAND 0x00443220
LEGO_EXPORT unsigned int GetArmColourOfBloke(struct BlokeSex0 *param_1) {
    unsigned int idx = param_1->field_4->field_90;
    unsigned int r = BlokeColours[idx * 3];
    unsigned int b = BlokeColours[idx * 3 + 2];
    unsigned int g = BlokeColours[idx * 3 + 1];
    return (((r << 8) | g) << 8) | b;
}

// FUNCTION: LEGOLAND 0x00443250
float SinFloat(float param_1) {
    return (float)sin(param_1);
}

// FUNCTION: LEGOLAND 0x00443260
float CosFloat(float param_1) {
    return (float)cos(param_1);
}

// FUNCTION: LEGOLAND 0x00443270
LEGO_EXPORT void MatrixMultiply(float *A, float *B, float *C) {
    C[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    C[1] = A[1] * B[4] + A[2] * B[7] + A[0] * B[1];
    C[2] = A[1] * B[5] + A[2] * B[8] + A[0] * B[2];

    C[3] = A[4] * B[3] + A[3] * B[0] + A[5] * B[6];
    C[4] = A[5] * B[7] + A[4] * B[4] + A[3] * B[1];
    C[5] = A[5] * B[8] + A[3] * B[2] + A[4] * B[5];

    C[6] = A[6] * B[0] + A[8] * B[6] + A[7] * B[3];
    C[7] = A[8] * B[7] + A[7] * B[4] + A[6] * B[1];
    C[8] = A[8] * B[8] + A[7] * B[5] + A[6] * B[2];
}

// FUNCTION: LEGOLAND 0x00443360
LEGO_EXPORT void BuildYRotationMatrix(float angle, float *out) {
    float s = SinFloat(angle);
    float c = CosFloat(angle);
    out[0] = c;
    out[6] = -s;
    out[1] = 0.0f;
    out[2] = s;
    out[8] = c;
    out[3] = 0.0f;
    out[4] = 1.0f;
    out[5] = 0.0f;
    out[7] = 0.0f;
}

// FUNCTION: LEGOLAND 0x004433b0
LEGO_EXPORT void TransformVectorsL(const int *src, int *dst, const int *m, int count) {
    /* Port [library:asm]: the original is inline asm (imul + shrd). Multiplies count 3-vectors by the 3x3 16.16
     * fixed-point matrix m. src and dst may be the same array. */
    int x;
    int y;
    int z;

    do {
        x = PortFixMul(m[0], src[0]) + PortFixMul(m[1], src[1]) + PortFixMul(m[2], src[2]);
        y = PortFixMul(m[3], src[0]) + PortFixMul(m[4], src[1]) + PortFixMul(m[5], src[2]);
        z = PortFixMul(m[6], src[0]) + PortFixMul(m[7], src[1]) + PortFixMul(m[8], src[2]);
        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
        src += 3;
        dst += 3;
    } while (--count != 0);
}

// FUNCTION: LEGOLAND 0x00443450
LEGO_EXPORT void NormaliseVector(struct Vec3 *v) {
    float m = (float)sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x = v->x / m;
    v->y = v->y / m;
    v->z = v->z / m;
}

struct Matrix3x3 {
    unsigned int m[9];
};

// FUNCTION: LEGOLAND 0x00443490
LEGO_EXPORT void CopyMatrix(struct Matrix3x3 *src, struct Matrix3x3 *dest) {
    dest->m[0] = src->m[0];
    dest->m[1] = src->m[1];
    dest->m[2] = src->m[2];
    dest->m[3] = src->m[3];
    dest->m[4] = src->m[4];
    dest->m[5] = src->m[5];
    dest->m[6] = src->m[6];
    dest->m[7] = src->m[7];
    dest->m[8] = src->m[8];
}
