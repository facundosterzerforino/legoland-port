#include "man3d.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloke.h"
#include "challenge.h"
#include "draw.h"
#include "globals.h"
#include "legoland.h"
#include "map_object.h"
#include "math.h"
#include "port_asm.h"
#include "print_sprite.h"
#include "render.h"
#include "render3d.h"
#include "resource.h"
#include "worker_mouse.h"

struct Mesh {
    /* 0x00 */ int count;
    /* 0x04 */ struct MeshElem *elems;
    /* 0x08 */ void *field_8;
};

// FUNCTION: LEGOLAND 0x0043f660
LEGO_EXPORT struct Position *LoadPos(const char *path) {
    struct ResFile *file;
    struct Position *pos;
    struct PosFrame *f;
    float *walk;
    float rot[3][3];
    float tmp[3][3];
    int i;
    int j;
    int row;
    int col;

    file = RES_OpenFile(path);
    pos = (struct Position *)malloc(0x28);
    pos->field_8 = 1.0f;
    pos->field_c = 1.0f;
    pos->field_10 = 1.0f;
    pos->field_14 = 0;
    pos->field_18 = 0;
    RES_ReadFile(file, &pos->count_inner, 4);
    RES_ReadFile(file, &pos->count, 4);
    pos->entries = (struct PosFrame **)malloc(pos->count << 2);
    i = 0;
    if (pos->count > 0) {
        do {
            pos->entries[i] = (struct PosFrame *)malloc(pos->count_inner * 0x30);
            j = 0;
            f = pos->entries[i];
            if (pos->count_inner > 0) {
                do {
                    RES_ReadFile(file, &f->pos[0], 4);
                    RES_ReadFile(file, &f->pos[1], 4);
                    RES_ReadFile(file, &f->pos[2], 4);
                    row = 3;
                    walk = &f->mat[0][0];
                    do {
                        col = 3;
                        do {
                            RES_ReadFile(file, walk, 4);
                            walk++;
                            col--;
                        } while (col != 0);
                        row--;
                    } while (row != 0);
                    BuildYRotationMatrix(1.5707963f, &rot[0][0]);
                    MatrixMultiply(&rot[0][0], &f->mat[0][0], &tmp[0][0]);
                    CopyMatrix((struct Matrix3x3 *)&tmp[0][0], (struct Matrix3x3 *)&f->mat[0][0]);
                    j++;
                    f++;
                } while (j < pos->count_inner);
            }
            i++;
        } while (i < pos->count);
    }
    RES_CloseFile(file);
    return pos;
}

// FUNCTION: LEGOLAND 0x0043f7d0
LEGO_EXPORT void UnloadPos(struct Position *pos) {
    int i;

    i = 0;
    if (pos->count > 0) {
        do {
            free(pos->entries[i]);
            i++;
        } while (i < pos->count);
    }
    free(pos->entries);
    free(pos);
}

// FUNCTION: LEGOLAND 0x0043f810
void FUN_0043f810(struct Person *person) {
    person->prev = 0;
    person->next = 0;
    if (DAT_00655a3c != 0) {
        person->next = DAT_00655a3c;
        ((struct Person *)DAT_00655a3c)->prev = person;
    }
    DAT_00655a3c = person;
}

// FUNCTION: LEGOLAND 0x0043f840
void FUN_0043f840(struct Person *person) {
    if (person->prev != 0) {
        person->prev->next = person->next;
    } else {
        DAT_00655a3c = person->next;
    }
    if (person->next != 0) {
        person->next->prev = person->prev;
    }
}

// FUNCTION: LEGOLAND 0x0043f870
void FUN_0043f870(struct Person *person) {
    if (person->field_50 != 0) {
        free(person->field_50);
    }
    free(person);
}

// FUNCTION: LEGOLAND 0x0043f890
LEGO_EXPORT struct Person *Find3DPersonFromBloke(struct Bloke *bloke) {
    struct Person *person;

    person = DAT_00655a3c;
    if (person == 0) {
        return 0;
    }
    while (person->bloke != bloke) {
        person = person->next;
        if (person == 0) {
            return 0;
        }
    }
    return person;
}

// FUNCTION: LEGOLAND 0x0043f8c0
struct Person *FUN_0043f8c0(struct Bloke *param_1, unsigned int param_2) {
    struct Person *person;

    person = malloc(0x94);
    if (person != 0) {
        memset(person, 0, 0x94);
        if (param_2 == 1) {
            person->random = rand() & 1;
        } else {
            person->random = 0;
        }
        person->field_8 = param_2;
        person->bloke = param_1;
        person->field_7c = 0xffffffff;
        person->field_80 = 0xffffffff;
        person->field_88 = 0xffffffff;
        person->field_90 = 0xffffffff;
        person->field_8c = 0xffffffff;
        person->field_10 = 0x40000000;
        person->field_14 = 0x40000000;
        person->field_18 = 0x40000000;
        person->prev = 0;
        person->next = 0;
        person->field_4c = 0;
        person->field_1c = 0;
        person->field_20 = 0;
        person->field_2c = 0;
        person->field_30 = 0;
        person->field_34 = 0xff;
        person->field_38 = 0;
    }
    return person;
}

// FUNCTION: LEGOLAND 0x0043f970
void FUN_0043f970(void *buffer) {
    struct Person *p = buffer;

    p->field_2c = p->field_2c + (unsigned int)p;
    p->field_30 = p->field_30 + (unsigned int)p;
}

// FUNCTION: LEGOLAND 0x0043f990
void *FUN_0043f990(const char *param_1, const char *param_2) {
    char path[256];
    struct ResFile *file;
    unsigned int size;
    void *buffer;

    // STRING: LEGOLAND 0x004b7b10
    sprintf(path, ".\\3ddata\\new\\%s\\%s", param_2, param_1);
    file = RES_OpenFile(path);
    if (file != 0) {
        size = RES_GetFileSize(file);
        buffer = malloc(size);
        if (buffer != 0) {
            RES_ReadFile(file, buffer, size);
            RES_CloseFile(file);
            FUN_0043f970(buffer);
            return buffer;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0043fa10
void FUN_0043fa10(float *param_1, int param_2) {
    int n;
    if (param_2 > 0) {
        n = param_2;
        do {
            if (*param_1 < FLOAT_004ab390) {
                *param_1 = 0.0f;
            }
            if (*param_1 > 1.0f) {
                *param_1 = 1.0f;
            }
            if (param_1[1] < FLOAT_004ab390) {
                param_1[1] = 0.0f;
            }
            if (param_1[1] > 1.0f) {
                param_1[1] = 1.0f;
            }
            param_1 = param_1 + 2;
            n = n - 1;
        } while (n != 0);
    }
}

// FUNCTION: LEGOLAND 0x0043fa80
void *FUN_0043fa80(const char *name, const char *dir, unsigned int ctx) { STUB(); }

// FUNCTION: LEGOLAND 0x0043fde0
void FUN_0043fde0(struct Mesh *mesh) {
    int count;
    int i;

    if (mesh != 0) {
        count = mesh->count;
        if (count > 0) {
            i = 0;
            do {
                free(mesh->elems[i].verts);
                free(mesh->elems[i].norms);
                i = i + 1;
                count = count - 1;
            } while (count != 0);
        }
        free(mesh->elems->shared->field_8);
        free(mesh->elems->shared);
        free(mesh->elems);
        free(mesh->field_8);
        free(mesh);
    }
}

// FUNCTION: LEGOLAND 0x0043fe50
LEGO_EXPORT void Render3DPerson(struct Person *person) {
    RECT bounds;
    RECT clip;
    struct VideoArg vid;
    unsigned int ptr;

    bounds.top = person->field_20;
    person->field_10 = 0x3f800000;
    person->field_14 = 0x3f800000;
    person->field_18 = 0x3f800000;
    bounds.left = person->field_1c;
    bounds.right = bounds.left + 0xa0;
    bounds.bottom = bounds.top + 0x78;
    clip.left = SPRITE_ClipRect.left;
    clip.top = SPRITE_ClipRect.top;
    clip.right = SPRITE_ClipRect.right - 1;
    clip.bottom = SPRITE_ClipRect.bottom - 1;
    if (IntersectRect(&clip, &bounds, &clip) != 0) {
        OffsetRect(&clip, -(int)person->field_1c, -(int)person->field_20);
        if (GetVideoSurface(&vid) != 0) {
            ptr = (unsigned int)vid.field_c + person->field_20 * vid.field_0 + person->field_1c * 2;
            FUN_00485f30(ptr, vid.field_0, vid.field_4, vid.field_8);
            FUN_00488700((unsigned int)vid.field_c, &DAT_00813a44);
            Render_SetViewport(&clip);
            __asm { fstcw word ptr [DAT_00638358] }
            __asm {fldcw word ptr[DAT_004b7abc]} FUN_00440a30(person);
            __asm { fldcw word ptr [DAT_00638358] }
            if (DAT_007feb14 != 0) {
                if (DAT_00668954 != 0 && person->bloke == FUN_004700f0()) {
                    return;
                }
                switch (person->field_8) {
                case 2:
                    Hover.type = 0x307;
                    Hover.ptr = person->bloke;
                    return;
                case 3:
                    Hover.type = 0x308;
                    Hover.ptr = person->bloke;
                    return;
                default:
                    Hover.type = 0x306;
                    Hover.ptr = person->bloke;
                }
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0043ffb0
LEGO_EXPORT void RenderBlokeIn3D(struct Bloke *bloke) {
    struct Person *person;

    person = bloke->person;
    if (person != 0) {
        Render3DPerson(person);
    }
}

// FUNCTION: LEGOLAND 0x0043ffd0
LEGO_EXPORT void SortBlokeIn3D(struct Bloke *bloke) {
    struct {
        unsigned int field_0;
        struct Bloke *bloke;
        unsigned short field_8;
    } info;

    info.field_0 = 0x306;
    info.bloke = bloke;
    info.field_8 = 0;
    if (bloke->person != 0) {
        SortPerson(bloke->person, bloke->person->sort_id, &info);
    }
}

// FUNCTION: LEGOLAND 0x00440010
LEGO_EXPORT void IP_RenderBlokeIn3DNow(struct Bloke *bloke) {
    RenderBlokeIn3D(bloke);
}

// FUNCTION: LEGOLAND 0x00440020
LEGO_EXPORT void SetPersonRotation(struct Person *person, float *src) {
    /* Port: the original builds the matrix in inline asm (fsin/fcos, fistp). Copies the rotation and builds
     * a 16.16 fixed-point rotation about the Y axis from src[1], with Y flipped. */
    float angle = src[1];
    int s;
    int c;

    person->field_40 = src[0];
    person->field_44 = src[1];
    person->field_48 = src[2];
    s = PortRound(sin(angle) * 65536.0f);
    c = PortRound(cos(angle) * 65536.0f);
    person->m[0] = c;
    person->m[1] = 0;
    person->m[2] = s;
    person->m[3] = 0;
    person->m[4] = -0x10000; /* the original stores 0x10000, then negates it */
    person->m[5] = 0;
    person->m[6] = -s;
    person->m[7] = 0;
    person->m[8] = c;
}

// FUNCTION: LEGOLAND 0x004400b0
LEGO_EXPORT void SetPersonDirection(struct Person *person, unsigned int direction) {
    person->field_48 = 0.0f;
    person->field_40 = 0.0f;
    switch (direction) {
    case 0:
        person->field_44 = -0.7853979468345642f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 1:
        person->field_44 = 4.712387561798096f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 2:
        person->field_44 = 3.926989793777466f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 3:
        person->field_44 = 3.141591787338257f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 4:
        person->field_44 = 2.356193780899048f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 5:
        person->field_44 = 1.5707958936691284f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 6:
        person->field_44 = 0.7853979468345642f;
        SetPersonRotation(person, &person->field_40);
        return;
    case 7:
        person->field_44 = 0.0f;
    }
    SetPersonRotation(person, &person->field_40);
}

// FUNCTION: LEGOLAND 0x00440190
LEGO_EXPORT void SetPersonPosition(struct Person *person, unsigned int x, unsigned int y) {
    person->field_1c = x;
    person->field_20 = y;
}

// FUNCTION: LEGOLAND 0x004401b0
void FUN_004401b0(struct Person *person, struct Bloke *bloke) {
    struct Point pt;
    int y;
    int x;
    short s;
    int w;
    int h;

    SetPersonDirection(person, bloke->field_72);
    y = bloke->pos.y;
    x = bloke->pos.x;
    GetTileDimensions(&w, &h);
    pt.x = (x - y) * w >> 9;
    pt.y = (y + x) * h >> 9;
    s = (short)Get_XScroll();
    pt.x -= s;
    s = (short)Get_YScroll();
    pt.y -= s;
    person->sort_id = pt.y;
    pt.x += lpConfig->field_20;
    pt.y += lpConfig->field_22 - (bloke->field_70 >> 1);
    AdjustBlokePosition(&pt);
    SetPersonPosition(person, pt.x, pt.y);
    if (!(bloke->flags & 0x100)) {
        person->field_4c = bloke->field_74;
    }
}

// FUNCTION: LEGOLAND 0x00440290
LEGO_EXPORT void UpdatePerson(Bloke *bloke) {
    if ((bloke->flags & 0x80) != 0) {
        return;
    }
    if (bloke->person == 0) {
        return;
    }
    FUN_004401b0(bloke->person, bloke);
}

// FUNCTION: LEGOLAND 0x004402b0
LEGO_EXPORT void Control3DPeople(void) {
    Bloke *bloke;

    bloke = FirstBloke;
    if (bloke == 0) {
        return;
    }
    do {
        UpdatePerson(bloke);
        bloke = bloke->next;
    } while (bloke != 0);
}

// FUNCTION: LEGOLAND 0x004402d0
void *FUN_004402d0(const char *param_1, const char *param_2) {
    char path[256];
    struct ResFile *file;
    unsigned int size;
    void *buffer;

    sprintf(path, ".\\3ddata\\new\\%s\\%s", param_1, param_2);
    file = RES_OpenFile(path);
    if (file != 0) {
        size = RES_GetFileSize(file);
        buffer = malloc(size);
        if (buffer != 0) {
            RES_ReadFile(file, buffer, size);
            RES_CloseFile(file);
        }
    }
    return buffer;
}

// FUNCTION: LEGOLAND 0x00440350
LEGO_EXPORT void InitMan(void) {
    unsigned int ctx;
    // STRING: LEGOLAND 0x004b7bb0
    const char *proj = "NewProject.txt";

    FUN_00485fc0((DAT_00668088 == 2) + 5);
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7cdc
    DAT_0081c8c0 = FUN_0043f990("NewProject.loc", "visitor");
    ((unsigned int *)DAT_0081c8c0)[1] = ctx;
    // STRING: LEGOLAND 0x004b7cec
    FUN_00443720(DAT_0081c8c0, "visitor");
    // STRING: LEGOLAND 0x004b7cc4
    DAT_0062fed4[1] = FUN_0043fa80("WomanWalk.WomanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7cac
    DAT_0062fed4[0] = FUN_0043fa80("WomanSit.WomanSit.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c94
    DAT_0062fed4[2] = FUN_0043fa80("WomanWave.WomanWave.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c78
    DAT_0062fed4[3] = FUN_0043fa80("WomanStand.WomanStand.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c5c
    DAT_0062fed4[5] = FUN_0043fa80("WomanPanWalk.WomPanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c44
    DAT_0062fed4[4] = FUN_0043fa80("WomanPan.WomanPan.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c30
    DAT_0062febc[1] = FUN_0043fa80("ManWalk.ManWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c1c
    DAT_0062febc[0] = FUN_0043fa80("ManSit.ManSit.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c08
    DAT_0062febc[2] = FUN_0043fa80("ManWave.ManWave.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bf0
    DAT_0062febc[3] = FUN_0043fa80("ManStand.ManStand.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bd4
    DAT_0062febc[5] = FUN_0043fa80("ManPanWalk.ManPanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bc0
    DAT_0062febc[4] = FUN_0043fa80("ManPan.ManPan.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7ba4
    FUN_00442980("altman.txt", proj, "visitor", 0, ctx);
    // STRING: LEGOLAND 0x004b7b94
    FUN_00442980("altwoman.txt", proj, "visitor", 1, ctx);
    DAT_00630100 = FUN_004402d0("visitor", "altman.txt");
    DAT_0062feac = FUN_004402d0("visitor", "altwoman.txt");
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7b80
    DAT_0081c8c8 = FUN_0043f990("geoff.loc", "geoff");
    ((unsigned int *)DAT_0081c8c8)[1] = ctx;
    // STRING: LEGOLAND 0x004b7b8c
    FUN_00443720(DAT_0081c8c8, "geoff");
    // STRING: LEGOLAND 0x004b7b68
    DAT_0062feb0[0] = FUN_0043fa80("geofWalk.GeofWalk.3d", "geoff", ctx);
    // STRING: LEGOLAND 0x004b7b50
    DAT_0062feb0[1] = FUN_0043fa80("GeofPour.GeofPour.3d", "geoff", ctx);
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7b3c
    DAT_0081c8c4 = FUN_0043f990("tracy.loc", "tracy");
    ((unsigned int *)DAT_0081c8c4)[1] = ctx;
    // STRING: LEGOLAND 0x004b7b48
    FUN_00443720(DAT_0081c8c4, "tracy");
    // STRING: LEGOLAND 0x004b7b24
    DAT_0062fef4 = FUN_0043fa80("TracyWalk.TraceWalk.3d", "tracy", ctx);
}

// FUNCTION: LEGOLAND 0x004405a0
LEGO_EXPORT void UnInitMan(void) {
    void **p;

    if (DAT_0081c8c0 != 0) {
        free(DAT_0081c8c0);
    }
    if (DAT_0081c8c8 != 0) {
        free(DAT_0081c8c8);
    }
    if (DAT_0081c8c4 != 0) {
        free(DAT_0081c8c4);
    }
    if (DAT_00630100 != 0) {
        free(DAT_00630100);
    }
    if (DAT_0062feac != 0) {
        free(DAT_0062feac);
    }
    p = DAT_0062febc;
    do {
        if (*p != 0) {
            FUN_0043fde0(*p);
        }
        p++;
    } while ((int)p < (int)DAT_0062fed4);
    p = DAT_0062fed4;
    do {
        if (*p != 0) {
            FUN_0043fde0(*p);
        }
        p++;
    } while ((int)p < (int)DAT_0062feec);
    p = DAT_0062feb0;
    do {
        if (*p != 0) {
            FUN_0043fde0(*p);
        }
        p++;
    } while ((int)p < (int)DAT_0062feb8);
    if (DAT_0062fef4 != 0) {
        FUN_0043fde0(DAT_0062fef4);
    }
    FUN_00442c70();
    FUN_00486250();
    FUN_004886a0();
    FUN_00485fa0();
}

// FUNCTION: LEGOLAND 0x00440680
LEGO_EXPORT void Add3DBlokeToList(struct Bloke *bloke, unsigned int param_2) {
    struct Person *person;

    person = FUN_0043f8c0(bloke, param_2);
    bloke->person = person;
    if (person != 0) {
        FUN_0043f810(person);
        FUN_004401b0(person, bloke);
        BlokeWalkAnim(bloke);
    }
}

// FUNCTION: LEGOLAND 0x004406c0
LEGO_EXPORT void BlokeSetAnim(struct Bloke *bloke, int anim) {
    struct Person *person;
    int kind;
    void **base;
    struct Mesh *mesh;
    void *context;

    person = bloke->person;
    if (person->field_88 != (unsigned int)anim) {
        kind = person->field_8;
        person->field_88 = anim;
        switch (kind) {
        case 1:
            if (person->random == 0) {
                base = DAT_0062febc;
            } else {
                base = DAT_0062fed4;
            }
            break;
        case 2:
            base = DAT_0062feb0;
            break;
        case 3:
            base = &DAT_0062fef4;
            break;
        }
        mesh = (struct Mesh *)base[anim];
        if (kind == 1 && person->field_50 != 0) {
            free(person->field_50);
        }
        switch (person->field_8) {
        case 1:
            context = DAT_0081c8c0;
            break;
        case 2:
            context = DAT_0081c8c8;
            break;
        case 3:
            context = DAT_0081c8c4;
            break;
        }
        {
            struct MeshElem *e = mesh->elems;
            unsigned int f8 = (unsigned int)mesh->field_8;
            person->field_50 = FUN_00442580(person, context, f8, e->shared->count, person->random);
        }
    }
}

// FUNCTION: LEGOLAND 0x00440780
LEGO_EXPORT void BlokeSitAnim(struct Bloke *bloke) {
    BlokeSetAnim(bloke, 0);
}

// FUNCTION: LEGOLAND 0x00440790
LEGO_EXPORT struct Anim3D *GetBlokeAnim3D(struct Bloke *bloke) {
    struct Person *person;
    struct Anim3D *result;
    void **base;

    result = 0;
    person = bloke->person;
    if (person != 0) {
        switch (person->field_8) {
        case 1:
            if (person->random == 0) {
                base = DAT_0062febc;
            } else {
                base = DAT_0062fed4;
            }
            break;
        case 2:
            base = DAT_0062feb0;
            break;
        case 3:
            base = &DAT_0062fef4;
            break;
        }
        result = (struct Anim3D *)base[person->field_88];
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00440800
LEGO_EXPORT struct Anim3D *GetBlokeAnim3DFromPerson(struct Person *person) {
    struct Anim3D *result;
    void **base;

    result = 0;
    if (person != 0) {
        switch (person->field_8) {
        case 1:
            if (person->random == 0) {
                base = DAT_0062febc;
            } else {
                base = DAT_0062fed4;
            }
            break;
        case 2:
            base = DAT_0062feb0;
            break;
        case 3:
            base = &DAT_0062fef4;
            break;
        }
        result = (struct Anim3D *)base[person->field_88];
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00440870
LEGO_EXPORT void BlokeSetFrame(struct Bloke *bloke, int frame) {
    struct Person *person;
    struct Anim3D *anim;

    person = bloke->person;
    if (person != 0) {
        anim = GetBlokeAnim3D(bloke);
        person->field_4c = frame % anim->divisor;
    }
}

// FUNCTION: LEGOLAND 0x004408a0
LEGO_EXPORT int PlayBlokeAnim(struct Bloke *bloke) {
    struct Person *person;
    struct Anim3D *anim;
    int frame;

    person = bloke->person;
    if (person != 0) {
        anim = GetBlokeAnim3DFromPerson(person);
        frame = person->field_4c + 1;
        person->field_4c = frame;
        if (anim->divisor <= frame) {
            person->field_4c = 0;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x004408e0
LEGO_EXPORT void BlokeAnimNextFrame(struct Bloke *bloke) {
    struct Person *person;
    struct Anim3D *anim;

    person = bloke->person;
    if (person != 0) {
        anim = GetBlokeAnim3D(bloke);
        person->field_4c = (person->field_4c + 1) % anim->divisor;
    }
}

// FUNCTION: LEGOLAND 0x00440910
LEGO_EXPORT void BlokeWalkAnim(struct Bloke *bloke) {
    int anim;

    switch (bloke->person->field_8) {
    case 2:
        anim = 0;
        break;
    case 3:
        anim = 0;
        break;
    default:
        anim = 1;
        break;
    }
    BlokeSetAnim(bloke, anim);
    BlokeSetFrame(bloke, 0);
}

// FUNCTION: LEGOLAND 0x00440960
LEGO_EXPORT void BlokeWalkWithPan(struct Bloke *bloke) {
    BlokeSetAnim(bloke, 5);
}

// FUNCTION: LEGOLAND 0x00440970
LEGO_EXPORT void BlokePanWithPan(struct Bloke *bloke) {
    BlokeSetAnim(bloke, 4);
}

struct IntVec3 {
    int x;
    int y;
    int z;
};

// FUNCTION: LEGOLAND 0x00440980
void FUN_00440980(struct MeshElem *elem, struct IntVec3 *out) {
    int n;
    int *verts;
    struct IntVec3 mn;
    struct IntVec3 mx;
    int vx;
    int vy;
    int vz;

    verts = (int *)elem->verts;
    mn.x = verts[0];
    mn.y = verts[1];
    mn.z = verts[2];
    mx = mn;
    verts = verts + 3;
    n = elem->vert_count;
    if (n > 1) {
        n = n - 1;
        do {
            vx = verts[0];
            vy = verts[1];
            vz = verts[2];
            if (vx < mn.x) {
                mn.x = vx;
            }
            if (vx > mx.x) {
                mx.x = vx;
            }
            if (vy < mn.y) {
                mn.y = vy;
            }
            if (vy > mx.y) {
                mx.y = vy;
            }
            if (vz < mn.z) {
                mn.z = vz;
            }
            if (vz > mx.z) {
                mx.z = vz;
            }
            verts = verts + 3;
        } while (--n != 0);
    }
    out[0] = mn;
    out[1] = mx;
}

// FUNCTION: LEGOLAND 0x00440a30
void FUN_00440a30(struct Person *person) { STUB(); }
