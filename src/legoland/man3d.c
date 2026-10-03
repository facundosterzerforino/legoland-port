#include "man3d.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../port/port_asm.h"
#include "bloke.h"
#include "challenge.h"
#include "draw.h"
#include "globals.h"
#include "legoland.h"
#include "map_object.h"
#include "math.h"
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
void AddPersonToList(struct Person *person) {
    person->prev = 0;
    person->next = 0;
    if (PersonListHead != 0) {
        person->next = PersonListHead;
        ((struct Person *)PersonListHead)->prev = person;
    }
    PersonListHead = person;
}

// FUNCTION: LEGOLAND 0x0043f840
void RemovePersonFromList(struct Person *person) {
    if (person->prev != 0) {
        person->prev->next = person->next;
    } else {
        PersonListHead = person->next;
    }
    if (person->next != 0) {
        person->next->prev = person->prev;
    }
}

// FUNCTION: LEGOLAND 0x0043f870
void FreePerson(struct Person *person) {
    if (person->field_50 != 0) {
        free(person->field_50);
    }
    free(person);
}

// FUNCTION: LEGOLAND 0x0043f890
LEGO_EXPORT struct Person *Find3DPersonFromBloke(struct Bloke *bloke) {
    struct Person *person;

    person = PersonListHead;
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
        person->character = param_2;
        person->bloke = param_1;
        person->field_7c = 0xffffffff;
        person->field_80 = 0xffffffff;
        person->anim = 0xffffffff;
        person->field_90 = 0xffffffff;
        person->field_8c = 0xffffffff;
        person->field_10 = 0x40000000;
        person->field_14 = 0x40000000;
        person->field_18 = 0x40000000;
        person->prev = 0;
        person->next = 0;
        person->frame = 0;
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
void RelocateLocData(void *buffer) {
    struct Person *p = buffer;

    p->field_2c = p->field_2c + (unsigned int)p;
    p->field_30 = p->field_30 + (unsigned int)p;
}

// FUNCTION: LEGOLAND 0x0043f990
void *LoadLocFile(const char *param_1, const char *param_2) {
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
            RelocateLocData(buffer);
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

/* One triangle of a mesh file (0x24 bytes). */
struct MeshFace {
    /* 0x00 */ unsigned int flags; /* bit 13 (0x2000): solid colour, otherwise textured */
    /* 0x04 */ unsigned char rgb[3];
    /* 0x07 */ unsigned char pad_7;
    /* 0x08 */ unsigned int texture; /* colour-cache id, or the texture id plus the caller's ctx */
    /* 0x0c */ float uv[6]; /* three (u, v) pairs */
};

// FUNCTION: LEGOLAND 0x0043fa80
void *FUN_0043fa80(const char *name, const char *dir, unsigned int ctx) {
    /* Port [library:asm]: the original loads this with C plus x87 float-to-int stores. Loads the mesh file
     * .\3ddata\new\<dir>\<name>: per element the vertices and normals are read, Y is flipped, the normals are
     * normalised and both are converted to 16.16 fixed point; then the shared index list and the faces. */
    char path[256];
    struct ResFile *file;
    struct Mesh *mesh;
    struct MeshShared *shared;
    struct MeshElem *elem;
    struct MeshFace *faces;
    float *v;
    int *iv;
    int count;
    int n;
    int e;
    int i;
    unsigned char rgb[3];

    sprintf(path, ".\\3ddata\\new\\%s\\%s", dir, name);
    mesh = 0;
    file = RES_OpenFile(path);
    if (file != 0) {
        mesh = (struct Mesh *)malloc(sizeof(struct Mesh));
        memset(mesh, 0, sizeof(struct Mesh));
        shared = (struct MeshShared *)malloc(sizeof(struct MeshShared));
        shared->count = 0;
        shared->field_4 = 0;
        shared->field_8 = 0;
        RES_ReadFile(file, &count, 4);
        mesh->count = count;
        mesh->elems = (struct MeshElem *)malloc(count * sizeof(struct MeshElem));
        memset(mesh->elems, 0, count * sizeof(struct MeshElem));
        for (e = 0; e < count; e++) {
            elem = &mesh->elems[e];
            RES_ReadFile(file, &n, 4);
            elem->vert_count = n;
            elem->verts = malloc(n * 12);
            RES_ReadFile(file, elem->verts, n * 12);
            v = (float *)elem->verts;
            for (i = 0; i < n; i++) {
                v[i * 3 + 1] = -v[i * 3 + 1];
            }
            iv = (int *)elem->verts;
            for (i = 0; i < n * 3; i++) {
                iv[i] = PortRound(v[i] * 65536.0f);
            }
            RES_ReadFile(file, &n, 4);
            elem->norm_count = n;
            elem->norms = malloc(n * 12);
            RES_ReadFile(file, elem->norms, n * 12);
            v = (float *)elem->norms;
            for (i = 0; i < n; i++) {
                NormaliseVector((struct Vec3 *)&v[i * 3]);
            }
            for (i = 0; i < n; i++) {
                v[i * 3 + 1] = -v[i * 3 + 1];
            }
            iv = (int *)elem->norms;
            for (i = 0; i < n * 3; i++) {
                iv[i] = PortRound(v[i] * 65536.0f);
            }
            elem->shared = shared;
            FUN_00440980(elem, (struct IntVec3 *)elem);
        }
        RES_ReadFile(file, &count, 4);
        RES_ReadFile(file, &shared->field_4, 4);
        shared->count = count;
        shared->field_8 = malloc(count * 12);
        RES_ReadFile(file, shared->field_8, count * 12);
        mesh->field_8 = malloc(count * sizeof(struct MeshFace));
        RES_ReadFile(file, mesh->field_8, count * sizeof(struct MeshFace));
        RES_CloseFile(file);
        faces = (struct MeshFace *)mesh->field_8;
        for (i = 0; i < count; i++) {
            if (faces[i].flags & 0x2000) {
                rgb[0] = faces[i].rgb[0];
                rgb[1] = faces[i].rgb[1];
                rgb[2] = faces[i].rgb[2];
                faces[i].texture = FUN_00486280(0x40, rgb);
            } else {
                faces[i].texture += ctx;
            }
            FUN_0043fa10(faces[i].uv, 3);
        }
    }
    return mesh;
}

// FUNCTION: LEGOLAND 0x0043fde0
void FreeMesh(struct Mesh *mesh) {
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
            ptr = (unsigned int)vid.bits + person->field_20 * vid.pitch + person->field_1c * 2;
            FUN_00485f30(ptr, vid.pitch, vid.width, vid.height);
            FUN_00488700((unsigned int)vid.bits, &MousePos);
            Render_SetViewport(&clip);
            __asm { fstcw word ptr [DAT_00638358] }
            __asm {fldcw word ptr[DAT_004b7abc]} FUN_00440a30(person);
            __asm { fldcw word ptr [DAT_00638358] }
            if (DAT_007feb14 != 0) {
                if (DAT_00668954 != 0 && person->bloke == GetWorkerOnMouse()) {
                    return;
                }
                switch (person->character) {
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
    /* Port [library:asm]: the original builds the matrix in inline asm (fsin/fcos, fistp). Copies the rotation and builds
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

    SetPersonDirection(person, bloke->dir);
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
    pt.x += lpConfig->view_x;
    pt.y += lpConfig->view_y - (bloke->height >> 1);
    AdjustBlokePosition(&pt);
    SetPersonPosition(person, pt.x, pt.y);
    if (!(bloke->flags & 0x100)) {
        person->frame = bloke->frame;
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
void *Load3DDataFile(const char *param_1, const char *param_2) {
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

    FUN_00485fc0((DisplayPixelFormat == 2) + 5);
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7cdc
    VisitorLocData = LoadLocFile("NewProject.loc", "visitor");
    ((unsigned int *)VisitorLocData)[1] = ctx;
    // STRING: LEGOLAND 0x004b7cec
    LoadTextureBitmaps(VisitorLocData, "visitor");
    // STRING: LEGOLAND 0x004b7cc4
    WomanMeshes[1] = FUN_0043fa80("WomanWalk.WomanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7cac
    WomanMeshes[0] = FUN_0043fa80("WomanSit.WomanSit.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c94
    WomanMeshes[2] = FUN_0043fa80("WomanWave.WomanWave.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c78
    WomanMeshes[3] = FUN_0043fa80("WomanStand.WomanStand.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c5c
    WomanMeshes[5] = FUN_0043fa80("WomanPanWalk.WomPanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c44
    WomanMeshes[4] = FUN_0043fa80("WomanPan.WomanPan.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c30
    ManMeshes[1] = FUN_0043fa80("ManWalk.ManWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c1c
    ManMeshes[0] = FUN_0043fa80("ManSit.ManSit.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7c08
    ManMeshes[2] = FUN_0043fa80("ManWave.ManWave.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bf0
    ManMeshes[3] = FUN_0043fa80("ManStand.ManStand.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bd4
    ManMeshes[5] = FUN_0043fa80("ManPanWalk.ManPanWalk.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7bc0
    ManMeshes[4] = FUN_0043fa80("ManPan.ManPan.3d", "visitor", ctx);
    // STRING: LEGOLAND 0x004b7ba4
    FUN_00442980("altman.txt", proj, "visitor", 0, ctx);
    // STRING: LEGOLAND 0x004b7b94
    FUN_00442980("altwoman.txt", proj, "visitor", 1, ctx);
    AltManFileData = Load3DDataFile("visitor", "altman.txt");
    AltWomanFileData = Load3DDataFile("visitor", "altwoman.txt");
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7b80
    GeoffLocData = LoadLocFile("geoff.loc", "geoff");
    ((unsigned int *)GeoffLocData)[1] = ctx;
    // STRING: LEGOLAND 0x004b7b8c
    LoadTextureBitmaps(GeoffLocData, "geoff");
    // STRING: LEGOLAND 0x004b7b68
    GeoffMeshes[0] = FUN_0043fa80("geofWalk.GeofWalk.3d", "geoff", ctx);
    // STRING: LEGOLAND 0x004b7b50
    GeoffMeshes[1] = FUN_0043fa80("GeofPour.GeofPour.3d", "geoff", ctx);
    ctx = FUN_00443710();
    // STRING: LEGOLAND 0x004b7b3c
    TracyLocData = LoadLocFile("tracy.loc", "tracy");
    ((unsigned int *)TracyLocData)[1] = ctx;
    // STRING: LEGOLAND 0x004b7b48
    LoadTextureBitmaps(TracyLocData, "tracy");
    // STRING: LEGOLAND 0x004b7b24
    TracyWalkMesh = FUN_0043fa80("TracyWalk.TraceWalk.3d", "tracy", ctx);
}

// FUNCTION: LEGOLAND 0x004405a0
LEGO_EXPORT void UnInitMan(void) {
    void **p;

    if (VisitorLocData != 0) {
        free(VisitorLocData);
    }
    if (GeoffLocData != 0) {
        free(GeoffLocData);
    }
    if (TracyLocData != 0) {
        free(TracyLocData);
    }
    if (AltManFileData != 0) {
        free(AltManFileData);
    }
    if (AltWomanFileData != 0) {
        free(AltWomanFileData);
    }
    p = ManMeshes;
    do {
        if (*p != 0) {
            FreeMesh(*p);
        }
        p++;
    } while (p < ManMeshes + 6);
    p = WomanMeshes;
    do {
        if (*p != 0) {
            FreeMesh(*p);
        }
        p++;
    } while (p < WomanMeshes + 6);
    p = GeoffMeshes;
    do {
        if (*p != 0) {
            FreeMesh(*p);
        }
        p++;
    } while (p < GeoffMeshes + 2);
    if (TracyWalkMesh != 0) {
        FreeMesh(TracyWalkMesh);
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
        AddPersonToList(person);
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
    if (person->anim != (unsigned int)anim) {
        kind = person->character;
        person->anim = anim;
        switch (kind) {
        case 1:
            if (person->random == 0) {
                base = ManMeshes;
            } else {
                base = WomanMeshes;
            }
            break;
        case 2:
            base = GeoffMeshes;
            break;
        case 3:
            base = &TracyWalkMesh;
            break;
        }
        mesh = (struct Mesh *)base[anim];
        if (kind == 1 && person->field_50 != 0) {
            free(person->field_50);
        }
        switch (person->character) {
        case 1:
            context = VisitorLocData;
            break;
        case 2:
            context = GeoffLocData;
            break;
        case 3:
            context = TracyLocData;
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
        switch (person->character) {
        case 1:
            if (person->random == 0) {
                base = ManMeshes;
            } else {
                base = WomanMeshes;
            }
            break;
        case 2:
            base = GeoffMeshes;
            break;
        case 3:
            base = &TracyWalkMesh;
            break;
        }
        result = (struct Anim3D *)base[person->anim];
    }
    return result;
}

// FUNCTION: LEGOLAND 0x00440800
LEGO_EXPORT struct Anim3D *GetBlokeAnim3DFromPerson(struct Person *person) {
    struct Anim3D *result;
    void **base;

    result = 0;
    if (person != 0) {
        switch (person->character) {
        case 1:
            if (person->random == 0) {
                base = ManMeshes;
            } else {
                base = WomanMeshes;
            }
            break;
        case 2:
            base = GeoffMeshes;
            break;
        case 3:
            base = &TracyWalkMesh;
            break;
        }
        result = (struct Anim3D *)base[person->anim];
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
        person->frame = frame % anim->divisor;
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
        frame = person->frame + 1;
        person->frame = frame;
        if (anim->divisor <= frame) {
            person->frame = 0;
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
        person->frame = (person->frame + 1) % anim->divisor;
    }
}

// FUNCTION: LEGOLAND 0x00440910
LEGO_EXPORT void BlokeWalkAnim(struct Bloke *bloke) {
    int anim;

    switch (bloke->person->character) {
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

static void PortSetVec(int *v, int x, int y, int z) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

/* One normal dotted with the light direction (16.16). A negative result becomes 1: the original shifts the
 * value right by (sign * 31), which leaves it unchanged when positive and gives 1 when negative. */
static int PortLightLevel(const int *normal, const int *light) {
    int dot = PortFixMul(normal[0], light[0]) + PortFixMul(normal[1], light[1]) + PortFixMul(normal[2], light[2]);

    if (dot < 0) {
        dot = 1;
    }
    return dot + 0x3333;
}

/* The mirrored case (negative parity) lists the triangle's vertices in reverse order, but the original
 * still gives vertex k the depth / texture coordinates / shade of index k (so these are not mirrored). */
static void PortProjectTriangle(const int *tri, int ox, int oy, int mirrored, struct PersonVertex *out) {
    int k;
    int j;
    int idx;
    int x;
    int y;

    for (k = 0; k < 3; k++) {
        /* isometric projection of the rotated, scaled vertex */
        idx = tri[k];
        x = DAT_00643ee8[idx][0];
        y = DAT_00643ee8[idx][1];
        j = mirrored ? 2 - k : k;
        out[j].x = ox + (DAT_00643ee8[idx][2] + x) * 2;
        out[j].y = y - x + DAT_00643ee8[idx][2] + oy;
        out[k].depth = DAT_00641004[tri[k]];
    }
}

// FUNCTION: LEGOLAND 0x00440a30
void FUN_00440a30(struct Person *person) {
    /* Port [library:asm]: the original is inline asm. Draws one 3D person (a Mesh element of the current
     * animation frame, see FUN_0043fa80) into the 160x120 sprite surface: rotates and scales the vertices,
     * projects them isometrically, shades them from a fixed light and a height / depth term, then fills each
     * front-facing triangle (solid colour or textured) with the triangle fillers in render.c. The first
     * shared->field_4 triangles have a normal per vertex (smooth shading), the rest one per triangle. */
    struct Anim3D *anim;
    struct MeshElem *elem;
    struct MeshShared *shared;
    const int *tris;
    const int *norms;
    const struct MeshFace *face;
    struct PersonVertex rec[3];
    int light_dir[3];
    int mt[9];
    int corners[8][3];
    int scaled[8][3];
    int scale[3];
    int light;
    int cx;
    int cz;
    int ox;
    int oy;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int min_z;
    int max_z;
    int zscale;
    int depth_scale;
    int divisor;
    int vcount;
    int tri_count;
    int smooth_count;
    int parity;
    int i;
    int k;
    const int *verts;
    float shade_f;

    anim = GetBlokeAnim3DFromPerson(person);
    elem = &anim->elems[person->frame];
    face = (const struct MeshFace *)person->field_50;
    shared = elem->shared;
    tri_count = shared->count;
    smooth_count = shared->field_4;
    tris = (const int *)shared->field_8;
    vcount = elem->vert_count;
    norms = (const int *)elem->norms;

    /* the person's scale, 16.16 (x and z are first multiplied by 0.447, the isometric foreshortening) */
    scale[0] = PortRound((float)(person->scale.x * 0.447f) * 65536.0f);
    scale[1] = PortRound(person->scale.y * 65536.0f);
    scale[2] = PortRound((float)(person->scale.z * 0.447f) * 65536.0f);

    /* shade_f == 0 is kept as the raw float bits (0), otherwise 16.16 shifted up by 8 */
    shade_f = person->field_38;
    if (shade_f == 0.0f) {
        memcpy(&light, &person->field_38, sizeof(light));
    } else {
        light = (int)((unsigned int)PortRound(shade_f * 65536.0f) << 8);
    }
    FUN_00485fe0(person->sprite, person->offset.x, person->offset.y);

    /* light direction, rotated into the person's frame (by the transposed orientation) */
    mt[0] = person->m[0];
    mt[1] = person->m[3];
    mt[2] = person->m[6];
    mt[3] = person->m[1];
    mt[4] = person->m[4];
    mt[5] = person->m[7];
    mt[6] = person->m[2];
    mt[7] = person->m[5];
    mt[8] = person->m[8];
    PortSetVec(light_dir, -0x1800, -0x5000, 0x3000);
    TransformVectorsL(light_dir, light_dir, mt, 1);

    /* centre of the element on x and z, negated */
    cx = -(elem->max_x + elem->min_x) >> 1;
    cz = -(elem->min_z + elem->max_z) >> 1;

    /* The element's bounding box corners, rotated and scaled, give the sprite's origin so the person ends up
     * centred. The original lists (min x, max y, max z) twice and never (max x, max y, min z); kept. */
    PortSetVec(corners[0], elem->min_x, elem->min_y, elem->min_z);
    PortSetVec(corners[1], elem->min_x, elem->min_y, elem->max_z);
    PortSetVec(corners[2], elem->min_x, elem->max_y, elem->min_z);
    PortSetVec(corners[3], elem->min_x, elem->max_y, elem->max_z);
    PortSetVec(corners[4], elem->max_x, elem->min_y, elem->min_z);
    PortSetVec(corners[5], elem->max_x, elem->min_y, elem->max_z);
    PortSetVec(corners[6], elem->min_x, elem->max_y, elem->max_z);
    PortSetVec(corners[7], elem->max_x, elem->max_y, elem->max_z);
    TransformVectorsL(&corners[0][0], &corners[0][0], person->m, 8);
    for (i = 0; i < 8; i++) {
        for (k = 0; k < 3; k++) {
            scaled[i][k] = PortFixMul(corners[i][k], scale[k]);
        }
    }
    min_x = max_x = scaled[0][0];
    min_y = max_y = scaled[0][1];
    for (i = 1; i < 8; i++) {
        if (scaled[i][0] < min_x) {
            min_x = scaled[i][0];
        }
        if (scaled[i][0] > max_x) {
            max_x = scaled[i][0];
        }
        if (scaled[i][1] < min_y) {
            min_y = scaled[i][1];
        }
        if (scaled[i][1] > max_y) {
            max_y = scaled[i][1];
        }
    }
    ox = 0x500000 - ((max_x - min_x) >> 1);
    oy = 0x5a0000 - ((max_y - min_y) >> 1);

    /* the vertices, moved to the element's centre / floor, then rotated */
    verts = (const int *)elem->verts;
    for (i = 0; i < vcount; i++) {
        DAT_00643ee8[i][0] = verts[i * 3] + cx;
        DAT_00643ee8[i][1] = verts[i * 3 + 1] - elem->min_y;
        DAT_00643ee8[i][2] = verts[i * 3 + 2] + cz;
    }
    TransformVectorsL(&DAT_00643ee8[0][0], &DAT_00643ee8[0][0], person->m, vcount);

    /* depth range of the rotated vertices */
    min_z = max_z = DAT_00643ee8[0][2];
    for (i = 0; i < vcount; i++) {
        if (DAT_00643ee8[i][2] < min_z) {
            min_z = DAT_00643ee8[i][2];
        }
        if (DAT_00643ee8[i][2] > max_z) {
            max_z = DAT_00643ee8[i][2];
        }
    }
    /* the original divides without a check (a flat element would crash it) */
    divisor = (max_z - min_z) >> 5;
    zscale = divisor != 0 ? 0x40000000 / divisor : 0;
    depth_scale = PortRound(person->depth * 65536.0f);

    /* per vertex: the depth/shade value stored in DAT_00641004, then the vertex is scaled */
    for (i = 0; i < vcount; i++) {
        int z = PortFixMul(DAT_00643ee8[i][2] - min_z, zscale);

        if (person->field_2c != 0) {
            z = PortFixMul(z, depth_scale);
        }
        DAT_00641004[i] = (person->field_34 << 24) + z + PortFixMul(DAT_00643ee8[i][1], light);
        for (k = 0; k < 3; k++) {
            DAT_00643ee8[i][k] = PortFixMul(DAT_00643ee8[i][k], scale[k]);
        }
    }

    /* The orientation is 16.16 ints here but TMNegParity reads floats (as in the original). m[4] (-0x10000)
     * read as a float is a NaN, so the parity test is unordered and TMNegParity returns 1, as on the x87. */
    parity = TMNegParity(person->fm);

    /* smooth-shaded triangles: three normals each (9 ints) */
    for (i = 0; i < smooth_count; i++) {
        int cross;

        PortProjectTriangle(&tris[i * 3], ox, oy, parity != 0 ? 0 : 1, rec);
        /* back-face test: skip if the screen-space cross product is not negative */
        cross = PortFixMul(rec[1].x - rec[0].x, rec[2].y - rec[0].y) - PortFixMul(rec[2].x - rec[0].x, rec[1].y - rec[0].y);
        if (cross < 0) {
            for (k = 0; k < 3; k++) {
                rec[k].shade = PortLightLevel(&norms[i * 9 + k * 3], light_dir);
            }
            if (face[i].flags & 0x2000) {
                FUN_004864e0(face[i].texture);
                FUN_00486590(&rec[0], &rec[1], &rec[2]);
            } else {
                for (k = 0; k < 3; k++) {
                    rec[k].u = face[i].uv[k * 2];
                    rec[k].v = face[i].uv[k * 2 + 1];
                }
                FUN_004886e0(face[i].texture);
                FUN_00486c70(&rec[0], &rec[1], &rec[2]);
            }
        }
    }

    /* flat-shaded triangles: one normal each, stored after the smooth ones' 36 bytes per triangle */
    i = smooth_count > 0 ? smooth_count : 0;
    norms = (const int *)((const char *)norms + i * 24);
    for (; i < tri_count; i++) {
        int cross;

        PortProjectTriangle(&tris[i * 3], ox, oy, parity != 0 ? 0 : 1, rec);
        cross = PortFixMul(rec[1].x - rec[0].x, rec[2].y - rec[0].y) - PortFixMul(rec[2].x - rec[0].x, rec[1].y - rec[0].y);
        if (cross < 0) {
            rec[0].shade = PortLightLevel(&norms[i * 3], light_dir);
            if (face[i].flags & 0x2000) {
                FUN_004864e0(face[i].texture);
                FUN_004877b0(&rec[0], &rec[1], &rec[2]);
            } else {
                for (k = 0; k < 3; k++) {
                    rec[k].u = face[i].uv[k * 2];
                    rec[k].v = face[i].uv[k * 2 + 1];
                }
                FUN_004886e0(face[i].texture);
                FUN_00487d40(&rec[0], &rec[1], &rec[2]);
            }
        }
    }
}
