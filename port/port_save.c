/* [port] Saved games in the original's 32-bit layout (port_save.h). */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/legoland/legoland.h"
#include "../src/legoland/llidb.h"
#include "port_save.h"
#include "port_trace.h"

/* the game's own functions, not the macros llidb.h makes of them */
#undef SaveGameRead
#undef SaveGameWrite

/* A size that isn't a whole number of the type's structs: the call reads or writes part of a struct, which this
   layer can't convert. tools/phase8/audit_save.py finds those calls before they run; this is the last net. */
static void PortSaveOdd(const char *what, unsigned int count, const struct PortX86Type *type) {
    PortTrace("save: %s of %u bytes of %s (0x%x each): not converted", what, count, type->name, type->size);
}

unsigned int PortSaveRead(void *buffer, unsigned int count, const struct PortX86Type *type) {
    unsigned char *raw;
    unsigned int n;
    unsigned int i;
    unsigned int ok;

    if (type == NULL || sizeof(void *) == 4) {
        return SaveGameRead(buffer, count);
    }
    if (count % type->size != 0) {
        PortSaveOdd("read", count, type);
        return SaveGameRead(buffer, count);
    }
    n = count / type->size;
    raw = (unsigned char *)malloc(type->size_x86 * (n ? n : 1));
    if (raw == NULL) {
        return 0;
    }
    ok = SaveGameRead(raw, type->size_x86 * n);
    if (ok) {
        for (i = 0; i < n; i++) {
            PortFromX86((char *)buffer + i * type->size, raw + i * type->size_x86, type->fields, type->count, 0,
                        type->size_x86);
        }
    }
    free(raw);
    return ok;
}

unsigned int PortSaveWrite(const void *buffer, unsigned int count, const struct PortX86Type *type) {
    unsigned char *raw;
    unsigned int n;
    unsigned int i;
    unsigned int ok;

    if (type == NULL || sizeof(void *) == 4) {
        return SaveGameWrite((void *)buffer, count);
    }
    if (count % type->size != 0) {
        PortSaveOdd("write", count, type);
        return SaveGameWrite((void *)buffer, count);
    }
    n = count / type->size;
    raw = (unsigned char *)malloc(type->size_x86 * (n ? n : 1));
    if (raw == NULL) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        PortToX86(raw + i * type->size_x86, (const char *)buffer + i * type->size, type);
    }
    ok = SaveGameWrite(raw, type->size_x86 * n);
    free(raw);
    return ok;
}

int PortSaveSelfTest(void *report) {
    FILE *f = (FILE *)report;
    int problems = 0;
    unsigned int t;

    for (t = 0; t < port_x86_type_count; t++) {
        const struct PortX86Type *type = port_x86_types[t];
        unsigned char *x86 = (unsigned char *)malloc(type->size_x86);
        unsigned char *back = (unsigned char *)malloc(type->size_x86);
        unsigned char *covered = (unsigned char *)calloc(type->size_x86, 1);
        unsigned char *native = (unsigned char *)calloc(type->size, 1);
        unsigned int i;
        unsigned int j;
        unsigned int bad = 0;
        unsigned int holes = 0;

        if (x86 == NULL || back == NULL || covered == NULL || native == NULL) {
            free(x86);
            free(back);
            free(covered);
            free(native);
            return problems + 1;
        }
        for (i = 0; i < type->size_x86; i++) {
            x86[i] = (unsigned char)(i * 37 + t * 11 + 1);
        }
        for (i = 0; i < type->count; i++) {
            const struct PortX86Field *fd = &type->fields[i];

            if (fd->src + fd->size > type->size_x86 || fd->dst + (fd->kind == 'B' ? fd->size : sizeof(void *)) > type->size) {
                fprintf(f, "save type %s: field at 0x%x out of bounds\n", type->name, fd->src);
                bad++;
                continue;
            }
            for (j = 0; j < fd->size; j++) {
                if (covered[fd->src + j]) {
                    fprintf(f, "save type %s: byte 0x%x in two fields\n", type->name, fd->src + j);
                    bad++;
                }
                covered[fd->src + j] = 1;
            }
        }
        PortFromX86(native, x86, type->fields, type->count, 0, type->size_x86);
        PortToX86(back, native, type);
        for (i = 0; i < type->size_x86; i++) {
            if (!covered[i]) {
                holes++;
            } else if (back[i] != x86[i]) {
                fprintf(f, "save type %s: byte 0x%x changes in the round trip\n", type->name, i);
                bad++;
                break;
            }
        }
        if (sizeof(void *) == 4 && type->size != type->size_x86) {
            fprintf(f, "save type %s: 0x%x bytes in this build, 0x%x in the original\n", type->name, type->size,
                    type->size_x86);
            bad++;
        }
        fprintf(f, "save type %s: %u fields, 0x%x bytes (0x%x here), %u padding bytes%s\n", type->name, type->count,
                type->size_x86, type->size, holes, bad ? " - PROBLEM" : "");
        problems += bad != 0;
        free(x86);
        free(back);
        free(covered);
        free(native);
    }
    return problems;
}
