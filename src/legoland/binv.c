#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "binv.h"

// FUNCTION: LEGOLAND 0x0044dc90
#ifdef LEGOLAND_PORT
/* [port] The file holds its frames and objects in the original 32-bit layout, linked by offsets from the start of
   the file, which the original turned into pointers in place. A 64-bit pointer doesn't fit those fields, so the
   frames and objects are built in the port's layout instead (on every build), in one block with a copy of the
   file after them: the vertices and names stay in that copy, and FreeBinV still frees everything at once. */
#define BINV_X86_HEADER 0x24 /* to frames_ofs, inclusive */
#define BINV_X86_FRAME 0xc
#define BINV_X86_OBJECT 0x34

static unsigned int BinVWord(const unsigned char *raw, unsigned int size, unsigned int off) {
    unsigned int v = 0;

    if (off <= size && size - off >= 4) {
        memcpy(&v, raw + off, 4);
    }
    return v;
}

LEGO_EXPORT BinVFile *LoadBinV(const char *filename) {
    FILE *stream;
    unsigned int size;
    unsigned char *raw;
    unsigned char *copy;
    BinVFile *file;
    BinVFrame *frames;
    BinVObject *objects;
    unsigned int nframes = 0;
    unsigned int nobjects = 0;
    unsigned int fo;
    unsigned int oo;
    unsigned int i;
    unsigned int j;
    unsigned int k;
    unsigned int count;

    stream = fopen(filename, "rb");
    if (stream == NULL) {
        return NULL;
    }
    fseek(stream, 0, SEEK_END);
    size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    raw = malloc(size);
    if (raw == NULL) {
        fclose(stream);
        return NULL;
    }
    fread(raw, 1, size, stream);
    fclose(stream);
    if (size < BINV_X86_HEADER || *(short *)raw != 0x101) {
        free(raw);
        return NULL;
    }
    /* count the frames and objects as LoadBinV walks them */
    fo = BinVWord(raw, size, 0x20);
    for (i = 0; i < *(unsigned short *)(raw + 2) && fo != 0; i++) {
        nframes++;
        count = BinVWord(raw, size, fo);
        oo = BinVWord(raw, size, fo + 4);
        for (j = 0; j < count && oo != 0; j++) {
            nobjects++;
            oo = BinVWord(raw, size, oo + 4);
        }
        fo = BinVWord(raw, size, fo + 8);
    }
    file = malloc(sizeof(BinVFile) + nframes * sizeof(BinVFrame) + nobjects * sizeof(BinVObject) + size);
    if (file == NULL) {
        free(raw);
        return NULL;
    }
    frames = (BinVFrame *)(file + 1);
    objects = (BinVObject *)(frames + nframes);
    copy = (unsigned char *)(objects + nobjects);
    memcpy(copy, raw, size);
    free(raw);
    memset(file, 0, sizeof(BinVFile));
    memcpy(file, copy, 0x20); /* magic, frame count, field_14: no pointers before frames */
    file->frames = nframes != 0 ? frames : NULL;
    fo = BinVWord(copy, size, 0x20);
    k = 0;
    for (i = 0; i < nframes; i++) {
        BinVFrame *frame = &frames[i];

        frame->count = (int)BinVWord(copy, size, fo);
        oo = BinVWord(copy, size, fo + 4);
        frame->objects = oo != 0 ? &objects[k] : NULL;
        for (j = 0; j < (unsigned int)frame->count && oo != 0; j++, k++) {
            BinVObject *object = &objects[k];
            unsigned int v = BinVWord(copy, size, oo + 8);
            unsigned int n = BinVWord(copy, size, oo + 0xc);

            object->count = (int)BinVWord(copy, size, oo);
            object->vertices = v != 0 ? (Vertex *)(copy + v) : NULL;
            object->name = n != 0 ? (char *)(copy + n) : NULL;
            memcpy(&object->m10, copy + oo + 0x10, 9 * sizeof(float));
            oo = BinVWord(copy, size, oo + 4);
            object->next = oo != 0 && k + 1 < nobjects ? &objects[k + 1] : NULL;
        }
        fo = BinVWord(copy, size, fo + 8);
        frame->next = fo != 0 && i + 1 < nframes ? &frames[i + 1] : NULL;
    }
    return file;
}
#else
LEGO_EXPORT BinVFile *LoadBinV(const char *filename) {
    FILE *stream;
    unsigned int size;
    union {
        BinVFile *file;
        unsigned int base;
    } mem;
    BinVFile *file;
    BinVFrame *frame;
    BinVObject *object;
    int i;
    int j;

    // STRING: LEGOLAND 0x004b81b8
    stream = fopen(filename, "rb");
    if (stream != NULL) {
        fseek(stream, 0, SEEK_END);
        size = ftell(stream);
        fseek(stream, 0, SEEK_SET);
        mem.file = malloc(size);
        file = mem.file;
        fread(file, 1, size, stream);
        fclose(stream);
        if (file->magic != 0x101) {
            free(file);
        } else {
            file->frames_ofs += mem.base;
            frame = file->frames;
            for (i = 0; i < file->frameCount; i++) {
                frame->objects_ofs += mem.base;
                object = frame->objects;
                for (j = 0; j < frame->count; j++) {
                    object->vertices_ofs += mem.base;
                    object->name_ofs += mem.base;
                    if (object->next != NULL) {
                        object->next_ofs += mem.base;
                        object = object->next;
                    }
                }
                if (frame->next != NULL) {
                    frame->next_ofs += mem.base;
                    frame = frame->next;
                }
            }
            return file;
        }
    }
    return NULL;
}
#endif

// FUNCTION: LEGOLAND 0x0044dd60
LEGO_EXPORT void FreeBinV(BinVFile *binv) {
    if (binv != NULL) {
        free(binv);
    }
}

// FUNCTION: LEGOLAND 0x0044dd70
LEGO_EXPORT BinVFrame *GetBinVFrame(BinVFile *file, int index) {
    BinVFrame *frame;
    int remaining;

    if (file == NULL) {
        return NULL;
    }
    frame = file->frames;
    if (index >= file->frameCount) {
        return NULL;
    }
    remaining = file->frameCount - 1;
    if (remaining > index) {
        remaining = remaining - index;
        do {
            frame = frame->next;
            remaining = remaining - 1;
        } while (remaining != 0);
    }
    return frame;
}

// FUNCTION: LEGOLAND 0x0044dda0
LEGO_EXPORT BinVObject *GetObjectFromName(BinVFrame *frame, const char *name) {
    BinVObject *object;
    int i;

    if (frame == NULL) {
        return NULL;
    }
    object = frame->objects;
    for (i = 0; i < frame->count; i++) {
        if (_stricmp(name, object->name) == 0) {
            break;
        }
        object = object->next;
    }
    return object;
}

// FUNCTION: LEGOLAND 0x0044ddf0
LEGO_EXPORT Vertex *GetVertex(BinVObject *object, int index) {
    if (object == NULL) {
        return NULL;
    }
    if (index >= object->count) {
        return NULL;
    }
    return object->vertices + index;
}

// FUNCTION: LEGOLAND 0x0044de20
LEGO_EXPORT float GetZSkew(BinVFile *file, BinVObject *object, Vertex *vertex) {
    return vertex->depth * vertex->depth / ((file->field_14 + file->field_14 - 1.0f) * vertex->depth - vertex->field_c * file->field_14);
}

// FUNCTION: LEGOLAND 0x0044de50
LEGO_EXPORT float GetUnitDepth(float near_z, float far_z) {
    float scale = 49152.0f / (near_z - far_z);
    float a = scale * (2.0 - far_z) + 8192.0;
    float b = scale * (1.0 - far_z) + 8192.0;

    return a - b;
}
