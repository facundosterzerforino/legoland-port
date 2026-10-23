/* [port] The resource checks of -port-selftest: the game's volumes open, and every object class's fixed part
   reads into the port's struct Ride (through PortFromX86 on every build). The fields are written to
   port-selftest-odf.txt in a form that doesn't depend on the build, so the 32-bit and the 64-bit reports must be
   the same file. */
#include <windows.h>
#include <ctype.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/legoland/binv.h"
#include "../src/legoland/bloke.h"
#include "../src/legoland/castle.h"
#include "../src/legoland/challenge.h"
#include "../src/legoland/draw.h"
#include "../src/legoland/gamemap.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/icon.h"
#include "../src/legoland/image_sprite.h"
#include "../src/legoland/interface.h"
#include "../src/legoland/legoland.h"
#include "../src/legoland/llidb.h"
#include "../src/legoland/man3d.h"
#include "../src/legoland/movie_pool.h"
#include "../src/legoland/obj_instance.h"
#include "../src/legoland/objclass.h"
#include "../src/legoland/print_sprite.h"
#include "../src/legoland/profile_io.h"
#include "../src/legoland/render.h"
#include "../src/legoland/render3d.h"
#include "../src/legoland/resource.h"
#include "../src/legoland/sound_sfx.h"
#include "../src/legoland/string.h"
#include "../src/legoland/text.h"
#include "../src/legoland/tilemap.h"
#include "../src/legoland/worker.h"
#include "miniwin/files.h"
#include "port_globals_layout.h"
#include "port_language.h"
#include "port_res.h"
#include "port_x86.h"

static FILE *PortResReport;
static FILE *PortResDump;
static int PortResProblems;
static int PortResClasses;

static void PortResCheckClass(const char *name, void *ctx) {
    char path[0x120];
    struct ResFile *file;
    unsigned int size = 0;
    unsigned int n;
    unsigned int from = PORT_X86_OFF_Ride_instances;
    unsigned char raw[PORT_X86_SIZE_Ride];
    struct Ride obj;
    unsigned int i;
    unsigned int j;

    (void)ctx;
    sprintf(path, "Objdesc\\%s", name);
    file = RES_OpenFile(path);
    if (file == NULL) {
        fprintf(PortResReport, "object class %s: can't open\n", name);
        PortResProblems++;
        return;
    }
    RES_ReadFile(file, &size, 4);
    n = size - 4;
    if (size < 4 || n > PORT_X86_SIZE_Ride - from) {
        fprintf(PortResReport, "object class %s: fixed part of %u bytes, struct Ride has 0x%x from its instances field\n",
            name, n, PORT_X86_SIZE_Ride - from);
        PortResProblems++;
        RES_CloseFile(file);
        return;
    }
    memset(raw, 0, sizeof(raw));
    RES_ReadFile(file, raw, n);
    memset(&obj, 0, sizeof(obj));
    PortFromX86(&obj, raw, port_x86_Ride, port_x86_Ride_count, from, from + n);
    fprintf(PortResDump, "%s", name);
    for (i = 0; i < port_x86_Ride_count; i++) {
        const struct PortX86Field *f = &port_x86_Ride[i];

        if (f->src < from || f->src + f->size > from + n) {
            continue;
        }
        if (f->kind == 'B') {
            fprintf(PortResDump, " %x:", f->src);
            for (j = 0; j < f->size; j++) {
                fprintf(PortResDump, "%02x", ((unsigned char *)&obj)[f->dst + j]);
            }
        } else {
            UINT_PTR v;

            memcpy(&v, (char *)&obj + f->dst, sizeof(v));
            fprintf(PortResDump, " %x=%x", f->src, (unsigned int)v);
        }
    }
    fprintf(PortResDump, "\n");
    PortResClasses++;
    RES_CloseFile(file);
}

/* FNV-1a over n bytes, for the dumps */
static unsigned int PortResHash(unsigned int h, const void *p, unsigned int n) {
    const unsigned char *b = (const unsigned char *)p;
    unsigned int i;

    for (i = 0; i < n; i++) {
        h = (h ^ b[i]) * 16777619u;
    }
    return h;
}

/* Every Zbuffers\*.bnv through LoadBinV: frames, objects (name, count) and a hash of each object's matrix and
   vertices, to port-selftest-bnv.txt. Returns the number of files read. */
static int PortResCheckBinV(FILE *dump) {
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA("Zbuffers\\*.bnv", &fd);
    int files = 0;

    if (h == INVALID_HANDLE_VALUE) {
        return 0;
    }
    do {
        char path[MAX_PATH];
        BinVFile *binv;
        BinVFrame *frame;
        int i;

        sprintf(path, "Zbuffers\\%s", fd.cFileName);
        binv = LoadBinV(path);
        if (binv == NULL) {
            FILE *f = MiniwinFopen(path, "rb"); /* a game path: backslashes, any case */
            short magic = 0;

            if (f != NULL) {
                fread(&magic, 2, 1, f);
                fclose(f);
            }
            if (magic != 0x101) {
                /* another version (octo.bnv): the original refuses it too, and nothing loads it */
                fprintf(dump, "%s skipped (version 0x%x)\n", fd.cFileName, (unsigned short)magic);
                continue;
            }
            fprintf(PortResReport, "binv %s: not loaded\n", fd.cFileName);
            PortResProblems++;
            continue;
        }
        fprintf(dump, "%s frames=%u z=%08x\n", fd.cFileName, binv->frameCount, PortResHash(2166136261u, &binv->field_14, 4));
        for (frame = binv->frames, i = 0; frame != NULL && i < binv->frameCount; frame = frame->next, i++) {
            BinVObject *object = frame->objects;
            int j;

            fprintf(dump, " frame %d: %d objects\n", i, frame->count);
            for (j = 0; j < frame->count && object != NULL; j++, object = object->next) {
                unsigned int hash = PortResHash(2166136261u, &object->m10, 9 * sizeof(float));

                if (object->vertices != NULL) {
                    hash = PortResHash(hash, object->vertices, object->count * sizeof(Vertex));
                }
                fprintf(dump, "  %s %d %08x\n", object->name != NULL ? object->name : "(none)", object->count, hash);
            }
        }
        FreeBinV(binv);
        files++;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return files;
}

/* The characters' .loc files through LoadLocFile: header, a hash of the texture table and every cell, to dump.
   Returns the number of files read. */
static int PortResCheckLoc(FILE *dump) {
    static const char *const files[][2] = {{"NewProject.loc", "visitor"}, {"geoff.loc", "geoff"}, {"tracy.loc", "tracy"}};
    int n = 0;
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        struct LocFile *loc = LoadLocFile(files[i][0], files[i][1]);

        if (loc == NULL) {
            fprintf(PortResReport, "loc %s: not loaded\n", files[i][0]);
            PortResProblems++;
            continue;
        }
        fprintf(dump, "%s count=%d base=%d cells=%d name=%.32s textures=%08x\n", files[i][0], loc->count,
            loc->texture_base, loc->cell_count, loc->name,
            PortResHash(2166136261u, loc->textures, loc->count * 8));
        for (j = 0; j < loc->cell_count; j++) {
            struct LocCell *cell = &loc->cells[j];

            fprintf(dump, " %d: %d %d,%d %dx%d\n", j, cell->texture, cell->x, cell->y, cell->w, cell->h);
        }
        free(loc);
        n++;
    }
    return n;
}

/* Every roller coaster mesh (CreatedData\*.lms) through LoadLmsFile: the header's counts, where each table is
   in the file, and hashes of the vertices, the three face lists and the box, to dump. Returns the files read. */
static int PortResCheckLms(FILE *dump) {
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA("RollerCoaster\\RollerCoaster\\CreatedData\\*.lms", &fd);
    int files = 0;

    if (h == INVALID_HANDLE_VALUE) {
        fprintf(PortResReport, "lms: no CreatedData\\*.lms\n");
        PortResProblems++;
        return 0;
    }
    do {
        char name[MAX_PATH];
        char *dot;
        struct LmsFile *lms;
        int count;

        strcpy(name, fd.cFileName);
        dot = strrchr(name, '.');
        if (dot != NULL) {
            *dot = 0;
        }
        lms = LoadLmsFile(name);
        if (lms == NULL) {
            fprintf(PortResReport, "lms %s: not loaded\n", fd.cFileName);
            PortResProblems++;
            continue;
        }
        memcpy(&count, lms->pad_0, 4);
        fprintf(dump, "%s verts=%d flat=%d smooth=%d textured=%d at %x %x %x %x %x %x\n", fd.cFileName, count,
            lms->field_1c, lms->field_24, lms->field_2c,
            (unsigned int)((unsigned char *)lms->field_c - (unsigned char *)(lms + 1)),
            (unsigned int)((unsigned char *)lms->field_10 - (unsigned char *)(lms + 1)),
            (unsigned int)((unsigned char *)lms->field_14 - (unsigned char *)(lms + 1)),
            (unsigned int)((unsigned char *)lms->field_18 - (unsigned char *)(lms + 1)),
            (unsigned int)((unsigned char *)lms->field_20 - (unsigned char *)(lms + 1)),
            (unsigned int)((unsigned char *)lms->field_28 - (unsigned char *)(lms + 1)));
        fprintf(dump, " %08x %08x %08x %08x %08x\n", PortResHash(2166136261u, lms->field_c, count * 12),
            PortResHash(2166136261u, lms->field_18, lms->field_1c * sizeof(struct LmsRef)),
            PortResHash(2166136261u, lms->field_20, lms->field_24 * sizeof(struct LmsRef)),
            PortResHash(2166136261u, lms->field_28, lms->field_2c * sizeof(struct LmsRef)),
            PortResHash(2166136261u, lms->box, sizeof(lms->box)));
        MemFree(lms);
        files++;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return files;
}

struct PortResMeshCtx {
    FILE *dump;
    const char *dir;
    int files;
};

static void PortResCheckMesh(const char *name, void *ctx) {
    struct PortResMeshCtx *c = (struct PortResMeshCtx *)ctx;
    size_t n = strlen(name);

    if (n < 3 || _stricmp(name + n - 3, ".3d") != 0) {
        return;
    }
    if (PortMeshDump(c->dump, name, c->dir)) {
        c->files++;
    } else {
        fprintf(PortResReport, "mesh %s\\%s: not loaded\n", c->dir, name);
        PortResProblems++;
    }
}

/* The 3D people's meshes (3DData\New\{visitor,geoff,tracy}\*.3d) through Load3DMesh, to dump. Returns the files
   read. */
static int PortResCheckMeshes(FILE *dump) {
    static const char *const dirs[] = {"visitor", "geoff", "tracy"};
    struct PortResMeshCtx c;
    char path[64];
    int i;

    c.dump = dump;
    c.files = 0;
    for (i = 0; i < 3; i++) {
        c.dir = dirs[i];
        sprintf(path, "3DData\\New\\%s\\", dirs[i]);
        PortResForEachFile(path, PortResCheckMesh, &c);
    }
    return c.files;
}

/* The sound effects: every wave file in the volumes' sfx\ made into a sample as the game makes them
 * (CreateSampleFromWAV: the volumes, the ACM's conversion to PCM, a DirectSound buffer), then deleted. */
static DWORD PortResStart;
static int PortResSfxLoaded;
static int PortResSfxWaves;

/* a wave file's format, for the report: "tag 2, 1 ch, 22050 Hz, 4 bits, block 512, 12345 data bytes" */
static void PortResWaveFormat(const char *path, char *out, size_t size) {
    struct ResFile *file = RES_OpenFile(path);
    unsigned int chunk[2];
    unsigned char fmt[16];
    int have_fmt = 0;

    snprintf(out, size, "no RIFF/WAVE header");
    if (file == NULL) {
        snprintf(out, size, "can't open");
        return;
    }
    if (RES_ReadFile(file, chunk, 8) == 8 && chunk[0] == 0x46464952 && RES_ReadFile(file, chunk, 4) == 4 &&
        chunk[0] == 0x45564157) {
        while (RES_ReadFile(file, chunk, 8) == 8) {
            if (chunk[0] == 0x20746d66 && chunk[1] >= 16) {
                unsigned int rest = chunk[1] - 16;
                char skip[64];

                if (RES_ReadFile(file, fmt, 16) != 16) {
                    break;
                }
                have_fmt = 1;
                while (rest > 0) {
                    unsigned int n = rest < sizeof(skip) ? rest : sizeof(skip);
                    RES_ReadFile(file, skip, n);
                    rest -= n;
                }
            } else if (chunk[0] == 0x61746164) {
                if (have_fmt) {
                    snprintf(out, size, "tag %u, %u ch, %u Hz, %u bits, block %u, %u data bytes",
                        fmt[0] | fmt[1] << 8, fmt[2] | fmt[3] << 8,
                        (unsigned int)(fmt[4] | fmt[5] << 8 | fmt[6] << 16 | (unsigned int)fmt[7] << 24),
                        fmt[14] | fmt[15] << 8, fmt[12] | fmt[13] << 8, chunk[1]);
                }
                break;
            } else {
                char skip[256];
                unsigned int rest = chunk[1] + (chunk[1] & 1);

                while (rest > 0) {
                    unsigned int n = rest < sizeof(skip) ? rest : sizeof(skip);
                    if (RES_ReadFile(file, skip, n) != (int)n) {
                        rest = 0;
                        break;
                    }
                    rest -= n;
                }
            }
        }
    }
    RES_CloseFile(file);
}

static void PortResCheckSfx(const char *name, void *ctx) {
    char path[MAX_PATH];
    char format[160];
    struct Sample *sample;
    size_t len = strlen(name);

    (void)ctx;
    if (len < 4 || _stricmp(name + len - 4, ".wav") != 0) {
        return; /* not a wave file (sfx\ also has the pirates' song, a .pk) */
    }
    sprintf(path, ".\\sfx\\%s", name);
    PortResSfxWaves++;
    sample = CreateSampleFromWAV(path);
    if (sample != NULL) {
        PortResSfxLoaded++;
        DeleteSampleDef(sample);
    } else {
        PortResWaveFormat(path, format, sizeof(format));
        fprintf(PortResReport, "sfx %s: doesn't load (%s)\n", name, format);
        PortResProblems++;
    }
}

static void PortResCheckSounds(void) {
    int files;

#ifdef LEGOLAND_WEB_NODE
    fprintf(PortResReport, "sfx: Node.js plays no sound, not checked\n");
    return;
#endif
    if (SoundAvailable == 0 && InitDirectSound(NULL) == 0) {
        fprintf(PortResReport, "sfx: no sound device, not checked\n");
        return;
    }
    PortResSfxLoaded = 0;
    PortResSfxWaves = 0;
    files = PortResForEachFile("sfx\\", PortResCheckSfx, NULL);
    if (files < 0) {
        fprintf(PortResReport, "sfx: no sfx directory\n");
        PortResProblems++;
        return;
    }
    fprintf(PortResReport, "sfx: %d of %d wave files made into samples\n", PortResSfxLoaded, PortResSfxWaves);
}

/* The music thread's start, timed: the game's own (StartMusicThread: DirectMusic's objects, the styles and segments
 * from imusic\, the synthesizer's instruments, the downloads), as at boot, where the game waits for it after the
 * logo movie. Then shut down. No sound device: not checked. */
static void PortResCheckMusicStart(void) {
    DWORD start;
    DWORD took;

#ifdef LEGOLAND_WEB_NODE
    fprintf(PortResReport, "music thread: Node.js plays no sound, not checked\n");
    return;
#endif
    if (SoundAvailable == 0) {
        fprintf(PortResReport, "music thread: no sound device, not checked\n");
        return;
    }
    MusicEnabled = 1;
    MusicThreadReady = 0;
    start = GetTickCount();
    StartMusicThread(NULL);
    while (MusicThreadReady == 0 && GetTickCount() - start < 120000) {
        Sleep(20);
    }
    took = GetTickCount() - start;
    if (MusicThreadReady == 0) {
        fprintf(PortResReport, "music thread: not started after %lu ms\n", (unsigned long)took);
        PortResProblems++;
        return;
    }
    fprintf(PortResReport, "music thread: started in %lu ms (DirectMusic %s)\n", (unsigned long)took,
        DMusicInitialised != 0 ? "ready" : "failed");
    if (DMusicInitialised == 0) {
        PortResProblems++;
    }
    ShutDownDirectMusic();
}

/* The people as the game draws them, to port-selftest-people.txt (the same as Windows' on every target): InitMan's
 * meshes and textures loaded (the display's pixel format as InitScreen leaves it: 16-bit, 5-6-5), each texture's
 * pixels and colour ramps checksummed, and a gardener's and a mechanic's faces as FUN_00442580 colours them: their
 * flags, solid colours and ramps. */
struct PortResRamp {
    int count;
    unsigned short *data; /* render.c's struct TextureFrame */
};

struct PortResMesh {
    int count;
    struct MeshElem *elems;
    struct PersonFace *faces; /* man3d.c's struct Mesh */
};

static unsigned int PortResFnv(const void *data, size_t size, unsigned int h) {
    const unsigned char *p = (const unsigned char *)data;
    size_t i;

    for (i = 0; i < size; i++) {
        h = (h ^ p[i]) * 16777619u;
    }
    return h;
}

static unsigned int PortResRampHash(const void *ramp) {
    const struct PortResRamp *r = (const struct PortResRamp *)ramp;

    return r == NULL || r->data == NULL ? 0 : PortResFnv(r->data, (size_t)r->count * 2, 2166136261u);
}

static void PortResDumpTextures(FILE *out, const char *who, struct LocFile *loc) {
    int i;

    if (loc == NULL) {
        fprintf(out, "%s: no textures\n", who);
        return;
    }
    for (i = 0; i < loc->count; i++) {
        struct TextureNode *t = (struct TextureNode *)DAT_00798190[loc->texture_base + i];
        unsigned int ramps = 2166136261u;
        int k;

        if (t == NULL) {
            fprintf(out, "%s texture %d: none\n", who, i);
            continue;
        }
        for (k = 0; k < 0x101; k++) {
            unsigned int h = PortResRampHash(t->data_c[k]);

            ramps = PortResFnv(&h, sizeof(h), PortResFnv(&k, sizeof(k), ramps));
        }
        fprintf(out, "%s texture %d: %dx%d pixels %08x ramps %08x\n", who, i, t->width_m1 + 1, t->height_m1 + 1,
            PortResFnv(t->data_8, (size_t)(t->width_m1 + 1) * (t->height_m1 + 1), 2166136261u), ramps);
    }
}

static void PortResDumpFaces(FILE *out, const char *who, int character, struct LocFile *loc, void *mesh) {
    struct PortResMesh *m = (struct PortResMesh *)mesh;
    struct Person person;
    struct PersonFace *faces;
    unsigned int count;
    unsigned int i;

    if (m == NULL || m->elems == NULL || m->elems->shared == NULL) {
        fprintf(out, "%s: no mesh\n", who);
        return;
    }
    memset(&person, 0, sizeof(person));
    person.character = character;
    count = (unsigned int)m->elems->shared->count;
    faces = (struct PersonFace *)FUN_00442580(&person, loc, m->faces, count, 0);
    if (faces == NULL) {
        fprintf(out, "%s: no faces\n", who);
        return;
    }
    for (i = 0; i < count; i++) {
        struct PersonFace *f = &faces[i];

        if ((f->flags & 0x2000) != 0) {
            const struct PortResRamp *r = (const struct PortResRamp *)f->ramp;

            fprintf(out, "%s face %u: flags %08x colour %02x %02x %02x ramp %08x (%04x %04x %04x)\n", who, i, f->flags,
                f->b0, f->b1, f->b2, PortResRampHash(f->ramp), r != NULL && r->data != NULL ? r->data[8] : 0,
                r != NULL && r->data != NULL ? r->data[32] : 0, r != NULL && r->data != NULL ? r->data[56] : 0);
        } else {
            fprintf(out, "%s face %u: flags %08x colour %02x %02x %02x cell %d\n", who, i, f->flags, f->b0, f->b1, f->b2,
                f->cell);
        }
    }
    free(faces);
}

static void PortResCheckPeople(void) {
    FILE *out = fopen("port-selftest-people.txt", "w");

    if (out == NULL) {
        return;
    }
    DisplayPixelFormat = 2; /* InitScreen's: 16-bit, 5-6-5 */
    InitMan();
    /* the colour table the ramps are made from (FUN_004860f0), and what it's made from */
    fprintf(out, "colours: green bits %u, constants %g %g, ColorLut[128] %04x %04x %04x, [255] %04x %04x %04x\n",
        *(unsigned int *)&DAT_007cb5e0, FLOAT_004ab550, FLOAT_004ab444, ColorLut[128].shifted, ColorLut[128].scaled,
        ColorLut[128].value, ColorLut[255].shifted, ColorLut[255].scaled, ColorLut[255].value);
    PortResDumpTextures(out, "visitor", VisitorLocData);
    PortResDumpTextures(out, "gardener", GeoffLocData);
    PortResDumpTextures(out, "mechanic", TracyLocData);
    PortResDumpFaces(out, "gardener", 2, GeoffLocData, GeoffMeshes[0]);
    PortResDumpFaces(out, "mechanic", 3, TracyLocData, TracyWalkMesh);
    fclose(out);
    fprintf(PortResReport, "people: textures and faces in port-selftest-people.txt\n");
}

/* The game's text as the game prints it: its own helpers (Print, PrintCent, PrintLimitedText...) on a DirectDraw
 * surface made the render target, as the screens print theirs (the notebook's objectives, the info panels), with the
 * game's fonts. The surface is filled with a marker colour first; the pixels each helper changed are counted. */
static LPDIRECTDRAWSURFACE PortResTextSurface;

static void PortResTextFill(void) {
    DDSURFACEDESC desc;
    int x;
    int y;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    if (PortResTextSurface->lpVtbl->Lock(PortResTextSurface, NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK) {
        return;
    }
    for (y = 0; y < (int)desc.dwHeight; y++) {
        for (x = 0; x < (int)desc.dwWidth; x++) {
            ((unsigned short *)((unsigned char *)desc.lpSurface + y * desc.lPitch))[x] = 0x1234;
        }
    }
    PortResTextSurface->lpVtbl->Unlock(PortResTextSurface, desc.lpSurface);
}

static int PortResTextCount(void) {
    DDSURFACEDESC desc;
    int x;
    int y;
    int n = 0;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    if (PortResTextSurface->lpVtbl->Lock(PortResTextSurface, NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK) {
        return -1;
    }
    for (y = 0; y < (int)desc.dwHeight; y++) {
        for (x = 0; x < (int)desc.dwWidth; x++) {
            n += ((unsigned short *)((unsigned char *)desc.lpSurface + y * desc.lPitch))[x] != 0x1234;
        }
    }
    PortResTextSurface->lpVtbl->Unlock(PortResTextSurface, desc.lpSurface);
    return n;
}

static void PortResCheckTextPath(void) {
    static const struct {
        HFONT *font;
        int height;
        int weight;
    } fonts[] = {{&LegoFont24Bold, 24, 700}, {&LegoFont28Normal, 28, 400}, {&LegoFont20Bold, 20, 700},
        {&LegoFont18SemiBold, 18, 600}};
    const char *line = "LEGOLAND Park 1999";
    DDSURFACEDESC desc;
    RECT rc = {4, 4, 316, 60};
    RECT saved_clip;
    LPDIRECTDRAWSURFACE saved_target;
    struct TextCell *cell;
    int counts[5];
    int cell_pixels = -1;
    size_t i;

    if (CheckHostSystemGPU() == 0) {
        fprintf(PortResReport, "text on a surface: no DirectDraw\n");
        PortResProblems++;
        return;
    }
    for (i = 0; i < sizeof(fonts) / sizeof(fonts[0]); i++) {
        if (*fonts[i].font == NULL) {
            LOGFONTA lf;

            memset(&lf, 0, sizeof(lf));
            lf.lfHeight = fonts[i].height;
            lf.lfWeight = fonts[i].weight;
            lf.lfCharSet = 1;
            lf.lfQuality = 2;
            strcpy(lf.lfFaceName, "Lego");
            *fonts[i].font = CreateFontIndirectA(&lf); /* InitScreen's fonts */
        }
    }
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.dwWidth = 320;
    desc.dwHeight = 64;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    if (DDRAWENV.ddraw2->lpVtbl->CreateSurface(DDRAWENV.ddraw2, &desc, &PortResTextSurface, NULL) != DD_OK) {
        fprintf(PortResReport, "text on a surface: no surface\n");
        PortResProblems++;
        return;
    }
    saved_clip = SPRITE_ClipRect;
    saved_target = renderEngine;
    renderEngine = PortResTextSurface;
    SetRect(&SPRITE_ClipRect, 0, 0, 320, 64);
    VideoSurfaceLocked = 0;
    PortResTextFill();
    Print(4, 4, line, 0);
    counts[0] = PortResTextCount();
    PortResTextFill();
    PrintCent(160, 4, 300, line, 1);
    counts[1] = PortResTextCount();
    PortResTextFill();
    PrintLimitedText(4, 4, 300, line, 2, RGB(0, 0, 0), 0x10);
    counts[2] = PortResTextCount();
    PortResTextFill();
    DrawTextOnRenderSurface((char *)line, 3, rc, RGB(255, 255, 255));
    counts[3] = PortResTextCount();
    PortResTextFill();
    FUN_00455220(4, 4, line, 2, 300);
    counts[4] = PortResTextCount();
    fprintf(PortResReport,
        "text on a surface: Print %d pixels, PrintCent %d, PrintLimitedText %d, DrawTextOnRenderSurface %d, "
        "FUN_00455220 %d\n",
        counts[0], counts[1], counts[2], counts[3], counts[4]);
    for (i = 0; i < 5; i++) {
        if (counts[i] <= 0) {
            PortResProblems++;
        }
    }
    /* a text cell, as BubbleHelp makes the advisor's speech bubble (the tutorial's messages): a function-based sprite
     * the sprite engine renders by FUN_00455a50 into its own surface; the pixels that aren't the background counted */
    cell = CreateTextCell("Build the Small Power Station in the park.", 200, 40, 0, 0x10, 0xd6dede, 0);
    if (cell != NULL && RecreateSprite(cell->sprite) && MakeSprite(cell->sprite) && cell->sprite->surface != NULL) {
        LPDIRECTDRAWSURFACE s = (LPDIRECTDRAWSURFACE)cell->sprite->surface;

        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        if (s->lpVtbl->Lock(s, NULL, &desc, DDLOCK_WAIT, NULL) == DD_OK) {
            unsigned short bg = *(unsigned short *)desc.lpSurface;
            int x;
            int y;

            cell_pixels = 0;
            for (y = 0; y < (int)desc.dwHeight; y++) {
                for (x = 0; x < (int)desc.dwWidth; x++) {
                    cell_pixels += ((unsigned short *)((unsigned char *)desc.lpSurface + y * desc.lPitch))[x] != bg;
                }
            }
            s->lpVtbl->Unlock(s, desc.lpSurface);
        }
    }
    fprintf(PortResReport, "text cell (the speech bubble's): %d pixels\n", cell_pixels);
    if (cell_pixels <= 0) {
        PortResProblems++;
    }
    FlushTextCells(1);
    renderEngine = saved_target;
    SPRITE_ClipRect = saved_clip;
    PortResTextSurface->lpVtbl->Release(PortResTextSurface);
    PortResTextSurface = NULL;
}

/* A sprite drawn scaled (PrintScaledSprite -> RenderScaledSpriteOffset: the map's scenery) partly outside its target, with the
 * game's clipper as the game in a window makes it (InitScreen: SetHWnd, and no clip list until the map's first render
 * sets one). miniwin refused such a Blt instead of clipping it, and the map's first render lost every scenery piece
 * that sticks out of its top or bottom (2026-10-17). */
static void PortResCheckScaledSprite(void) {
    DDSURFACEDESC desc;
    LPDIRECTDRAWSURFACE target = NULL;
    LPDIRECTDRAWSURFACE saved_target;
    LPDIRECTDRAWCLIPPER made = NULL;
    RECT saved_clip;
    struct Sprite *sprite;
    int counts[2] = {-1, -1};
    int i;

    sprite = LoadSprite("mapSpanner.lls", 0);
    if (sprite == NULL || CheckHostSystemGPU() == 0) {
        fprintf(PortResReport, "scaled sprite: no sprite or no DirectDraw\n");
        PortResProblems++;
        return;
    }
    if (DDrawClipper == NULL) {
        /* the window itself doesn't matter (the game's, InitScreen's SetHWnd), only that the clipper has one */
        DDRAWENV.ddraw2->lpVtbl->CreateClipper(DDRAWENV.ddraw2, 0, &made, NULL);
        made->lpVtbl->SetHWnd(made, 0, (HWND)(INT_PTR)1);
        DDrawClipper = made;
    }
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.dwWidth = 640; /* the map's picture (InitMapScreen) */
    desc.dwHeight = 340;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    if (DDRAWENV.ddraw2->lpVtbl->CreateSurface(DDRAWENV.ddraw2, &desc, &target, NULL) != DD_OK) {
        fprintf(PortResReport, "scaled sprite: no surface\n");
        PortResProblems++;
        KillSprite(sprite);
        return;
    }
    saved_clip = SPRITE_ClipRect;
    saved_target = renderEngine;
    renderEngine = target;
    SetRect(&SPRITE_ClipRect, 0, 0, 640, 340);
    for (i = 0; i < 2; i++) {
        int w = sprite->width * 2;
        int h = sprite->height * 2;
        int x;
        int y;

        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        if (target->lpVtbl->Lock(target, NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK) {
            break;
        }
        for (y = 0; y < (int)desc.dwHeight; y++) {
            memset((unsigned char *)desc.lpSurface + y * desc.lPitch, 0, desc.dwWidth * 2);
        }
        target->lpVtbl->Unlock(target, desc.lpSurface);
        /* inside, then half above the top edge, as the scenery at the top of the map */
        PrintScaledSprite(sprite, 100, i == 0 ? 50 : -h / 2, w, h);
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        if (target->lpVtbl->Lock(target, NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK) {
            break;
        }
        counts[i] = 0;
        for (y = 0; y < (int)desc.dwHeight; y++) {
            for (x = 0; x < (int)desc.dwWidth; x++) {
                counts[i] += ((unsigned short *)((unsigned char *)desc.lpSurface + y * desc.lPitch))[x] != 0;
            }
        }
        target->lpVtbl->Unlock(target, desc.lpSurface);
    }
    fprintf(PortResReport, "scaled sprite (the map's scenery): %d pixels inside the target, %d half outside it\n",
        counts[0], counts[1]);
    if (counts[0] <= 0 || counts[1] <= 0) {
        PortResProblems++;
    }
    renderEngine = saved_target;
    SPRITE_ClipRect = saved_clip;
    target->lpVtbl->Release(target);
    if (made != NULL) {
        DDrawClipper = NULL;
        made->lpVtbl->Release(made);
    }
    KillSprite(sprite);
}

/* The string table as the game loads it (LoadStringTable), against strings\\stab.str read here: every id the file
 * has must come back from GetString, the same text. A missing one is a text missing in the game (the exit dialog,
 * the tutorial notebook, the info popups' "(null)"). */
static void PortResCheckStrings(void) {
    FILE *f = fopen("strings/stab.str", "rb");
    char *data;
    long size;
    long i;
    int ids = 0;
    int found = 0;
    int differ = 0;
    int reported = 0;

    if (f == NULL || fseek(f, 0, SEEK_END) != 0 || (size = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET) != 0 ||
        (data = (char *)malloc((size_t)size + 1)) == NULL || fread(data, 1, (size_t)size, f) != (size_t)size) {
        if (f != NULL) {
            fclose(f);
        }
        fprintf(PortResReport, "strings: strings/stab.str doesn't read\n");
        PortResProblems++;
        return;
    }
    fclose(f);
    data[size] = 0;
    {
        /* LoadStringTable's own steps, replayed: which fails where the table comes out empty */
        char cwd[256];
        char token[0xf0];
        FILE *g;
        int total = 0;
        int len;
        int got = -1;
        int chdir_result = -1;
        int seek_result = -1;
        char *again;

        if (_getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "(none)");
        }
        g = MiniwinFopen(".\\strings\\stab.str", "r");
        if (g != NULL) {
            while (!feof(g)) {
                len = (int)fread(token, 1, sizeof(token), g);
                if (ferror(g)) {
                    break;
                }
                total += len;
            }
            seek_result = fseek(g, 0, 0);
            again = (char *)malloc((size_t)total + 1);
            got = again != NULL ? (int)fread(again, 1, (size_t)total, g) : -1;
            free(again);
            fclose(g);
        }
        chdir_result = _chdir(cwd);
        fprintf(PortResReport, "strings: the game's steps: cwd %s, fopen %s, %d bytes read (file %ld), fseek %d, read again %d, "
                               "_chdir %d\n",
            cwd, g != NULL ? "ok" : "failed", total, size, seek_result, got, chdir_result);
    }
    LoadStringTable();
    for (i = 0; i < size;) {
        if (isdigit((unsigned char)data[i])) {
            int id = 0;
            char text[1024];
            int n = 0;
            const char *got;

            while (i < size && isdigit((unsigned char)data[i])) {
                id = id * 10 + (data[i++] - '0');
            }
            while (i < size && data[i] != '"' && !isdigit((unsigned char)data[i])) {
                i++;
            }
            if (i >= size || data[i] != '"') {
                continue;
            }
            i++;
            while (i < size) {
                if (data[i] == '"') {
                    if (i + 1 < size && data[i + 1] == '"') {
                        i += 2;
                        if (n < (int)sizeof(text) - 1) {
                            text[n++] = '"';
                        }
                        continue;
                    }
                    i++;
                    break;
                }
                if (n < (int)sizeof(text) - 1) {
                    text[n++] = data[i];
                }
                i++;
            }
            text[n] = 0;
            ids++;
            got = GetString(id);
            if (got != NULL) {
                found++;
                if (strcmp(got, text) != 0) {
                    differ++;
                }
            } else if (reported++ < 5) {
                fprintf(PortResReport, "strings: id %d (\"%.40s\") missing\n", id, text);
            }
        } else {
            i++;
        }
    }
    DeleteStrings();
    free(data);
    fprintf(PortResReport, "strings: %d of %d ids in strings\\stab.str loaded, %d with another text\n", found, ids, differ);
    if (found != ids) {
        PortResProblems++;
    }
}

/* The globals the original has next to each other, next to each other here: the decompiled code walks from one into
 * the next in places (the map screen's 32 marker rows, the string table's slots) and reads past the ones declared
 * smaller than the original's data. A pair apart here is reported (the first few); port_globals_layout.c is made by
 * tools/port/gen_globals_layout.py. */
static void PortResCheckLayout(void) {
    FILE *all = fopen("port-selftest-layout.txt", "w"); /* every pair apart here */
    int i;
    int adjacent = 0;
    int apart = 0;

    for (i = 0; i + 1 < PortLayoutGlobalCount; i++) {
        const struct PortLayoutGlobal *a = &PortLayoutGlobals[i];
        const struct PortLayoutGlobal *b = &PortLayoutGlobals[i + 1];

        if (b->original - a->original != a->size) {
            continue; /* not next to each other in the original (or not with this size) */
        }
        adjacent++;
        if ((const char *)b->address != (const char *)a->address + a->size) {
            if (all != NULL) {
                fprintf(all, "%s %s\n", a->name, b->name);
            }
            if (apart++ < 8) {
                fprintf(PortResReport, "layout: %s and %s, next to each other in the original, %ld bytes apart here\n",
                    a->name, b->name, (long)((const char *)b->address - ((const char *)a->address + a->size)));
            }
        }
    }
    fprintf(PortResReport, "layout: %d of %d pairs of globals next to each other as in the original\n", adjacent - apart,
        adjacent);
    if (all != NULL) {
        fclose(all);
    }
}

int PortResSelfTest(const char *path) {
    int i;
    int files;

    PortResReport = fopen(path, "a");
    if (PortResReport == NULL) {
        return 1;
    }
    PortResStart = GetTickCount();
    PortResDump = fopen("port-selftest-odf.txt", "w");
    PortResProblems = 0;
    PortResClasses = 0;
    for (i = 0; i < 3; i++) {
        if (ResourceVolumes[i] == NULL) {
            ResourceVolumes[i] = RES_OpenVolume(ResourceFileNames[i]);
        }
        if (ResourceVolumes[i] == NULL) {
            fprintf(PortResReport, "resource volume %s: can't open\n", ResourceFileNames[i]);
            PortResProblems++;
        }
    }
    files = PortResDump != NULL ? PortResForEachFile("Objdesc\\", PortResCheckClass, NULL) : -1;
    if (files < 0) {
        fprintf(PortResReport, "resources: no Objdesc directory\n");
        PortResProblems++;
    }
    fprintf(PortResReport, "resources: %d object classes read of %d (fields in port-selftest-odf.txt)\n", PortResClasses,
        files);
    /* the CD's language (port_language.h): the European CDs' string 9999, which picks the object texts */
    LoadStringTable();
    fprintf(PortResReport, "language: %s\n",
        PortGameLanguage() != NULL ? PortGameLanguage() : "english (the English CD: no string 9999)");
    DeleteStrings();
    {
        FILE *bnv = fopen("port-selftest-bnv.txt", "w");

        if (bnv != NULL) {
            fprintf(PortResReport, "binv: %d files read (port-selftest-bnv.txt)\n", PortResCheckBinV(bnv));
            fclose(bnv);
        }
    }
    {
        FILE *loc = fopen("port-selftest-loc.txt", "w");

        if (loc != NULL) {
            fprintf(PortResReport, "loc: %d files read (port-selftest-loc.txt)\n", PortResCheckLoc(loc));
            fclose(loc);
        }
    }
    {
        FILE *lms = fopen("port-selftest-lms.txt", "w");

        if (lms != NULL) {
            fprintf(PortResReport, "lms: %d files read (port-selftest-lms.txt)\n", PortResCheckLms(lms));
            fclose(lms);
        }
    }
    {
        FILE *mesh = fopen("port-selftest-mesh.txt", "w");

        if (mesh != NULL) {
            fprintf(PortResReport, "meshes: %d files read (port-selftest-mesh.txt)\n", PortResCheckMeshes(mesh));
            fclose(mesh);
        }
    }
    /* the time the volumes took to read (small reads, as the game makes them: a browser's storage is slow at those) */
    fprintf(PortResReport, "resources: read in %lu ms\n", (unsigned long)(GetTickCount() - PortResStart));
    PortResCheckLayout();
    PortResCheckStrings();
    PortResCheckPeople();
    PortResCheckTextPath();
    PortResCheckScaledSprite();
    PortResCheckSounds();
    PortResCheckMusicStart();
    if (PortResDump != NULL) {
        fclose(PortResDump);
    }
    fclose(PortResReport);
    return PortResProblems;
}
