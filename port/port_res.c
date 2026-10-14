/* [port] The resource checks of -port-selftest: the game's volumes open, and every object class's fixed part
   reads into the port's struct Ride (through PortFromX86 on every build). The fields are written to
   port-selftest-odf.txt in a form that doesn't depend on the build, so the 32-bit and the 64-bit reports must be
   the same file. */
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../src/legoland/legoland.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/obj_instance.h"
#include "../src/legoland/resource.h"
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
    if (PortResDump != NULL) {
        fclose(PortResDump);
    }
    fclose(PortResReport);
    return PortResProblems;
}
