/* [port] The resource checks of -port-selftest: the game's volumes open, and every object class's fixed part
   reads into the port's struct Ride (through PortFromX86 on every build). The fields are written to
   port-selftest-odf.txt in a form that doesn't depend on the build, so the 32-bit and the 64-bit reports must be
   the same file. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/legoland/binv.h"
#include "../src/legoland/castle.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/legoland.h"
#include "../src/legoland/man3d.h"
#include "../src/legoland/movie_pool.h"
#include "../src/legoland/obj_instance.h"
#include "../src/legoland/profile_io.h"
#include "../src/legoland/resource.h"
#include "../src/legoland/sound_sfx.h"
#include "../src/legoland/string.h"
#include "miniwin/files.h"
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
        FUN_004775d0(lms);
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
    DAT_007988bc = 0;
    start = GetTickCount();
    StartMusicThread(NULL);
    while (DAT_007988bc == 0 && GetTickCount() - start < 120000) {
        Sleep(20);
    }
    took = GetTickCount() - start;
    if (DAT_007988bc == 0) {
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

int PortResSelfTest(const char *path) {
    int i;
    int files;

    PortResReport = fopen(path, "a");
    if (PortResReport == NULL) {
        return 1;
    }
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
    PortResCheckSounds();
    PortResCheckMusicStart();
    if (PortResDump != NULL) {
        fclose(PortResDump);
    }
    fclose(PortResReport);
    return PortResProblems;
}
