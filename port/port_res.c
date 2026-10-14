/* [port] The resource checks of -port-selftest: the game's volumes open, and every object class's fixed part
   reads into the port's struct Ride (through PortFromX86 on every build). The fields are written to
   port-selftest-odf.txt in a form that doesn't depend on the build, so the 32-bit and the 64-bit reports must be
   the same file. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/legoland/legoland.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/obj_instance.h"
#include "../src/legoland/binv.h"
#include "../src/legoland/resource.h"
#include "../src/legoland/man3d.h"
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
            FILE *f = fopen(path, "rb");
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
    if (PortResDump != NULL) {
        fclose(PortResDump);
    }
    fclose(PortResReport);
    return PortResProblems;
}
