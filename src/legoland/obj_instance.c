#include "obj_instance.h"
#include <stdlib.h>
#include "bloke.h"
#include "bloke_ai.h"
#include "globals.h"
#include "legoland.h"
#include "llidb.h"
#include "man3d.h"
#include "objclass.h"
#include "resource.h"
#include "sound_music.h"

struct InstanceNode {
    struct InstanceNode *prev;
    struct InstanceNode *next;
    struct InstanceList *list;
};

struct InstanceList {
    unsigned int pad_0;
    struct InstanceNode *next;
};

// FUNCTION: LEGOLAND 0x00489e60
char *ReadResFileLine(struct ResFile *file, char *dest, int maxlen) {
    int error = 0;
    int count = 0;
    char c;
    char *out = dest;
    while (1) {
        if (RES_ReadFile(file, &c, 1) == 0) {
            error = 1;
            break;
        }
        if (c == '\r') {
            break;
        }
        if (c == '\n') {
            goto done;
        }
        *out++ = c;
        count++;
        if (count >= maxlen) {
            break;
        }
    }
    if (c == '\r') {
        RES_ReadFile(file, &c, 1);
    }
done:
    *out = '\0';
    if (error && count == 0) {
        return 0;
    }
    return dest;
}

// FUNCTION: LEGOLAND 0x00489ee0
void FUN_00489ee0(void) {
    struct ObjTableEntry *entry;

    for (entry = ObjInstanceTable; entry < ObjInstanceTable + 128; entry++) {
        entry->key = 0xffff;
    }
}

// FUNCTION: LEGOLAND 0x00489f00
int FUN_00489f00(const struct Point *pos) {
    int index;
    struct ObjTableEntry *entry;

    index = 0;
    for (entry = ObjInstanceTable; entry < ObjInstanceTable + 128; entry++) {
        if (entry->key == 0xffff) {
            ObjInstanceTable[index].key = (pos->x << 8) + pos->y;
            ObjInstanceTable[index].value = 0;
            return 1;
        }
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489f50
int FUN_00489f50(const struct Point *pos) {
    int index;
    unsigned short target;
    struct ObjTableEntry *entry;

    index = 0;
    target = (pos->x << 8) + pos->y;
    for (entry = ObjInstanceTable; entry < ObjInstanceTable + 128; entry++) {
        if (entry->key == target) {
            ObjInstanceTable[index].key = 0xffff;
            return 1;
        }
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489f90
int FUN_00489f90(const struct Point *pos) {
    int index;
    unsigned short target;
    struct ObjTableEntry *entry;

    index = 0;
    target = (pos->x << 8) + pos->y;
    for (entry = ObjInstanceTable; entry < ObjInstanceTable + 128; entry++) {
        if (entry->key == target) {
            ObjInstanceTable[index].value++;
            return 1;
        }
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00489fd0
unsigned short FUN_00489fd0(const struct Point *pos) {
    int index;
    unsigned short target;
    struct ObjTableEntry *entry;

    index = 0;
    target = (pos->x << 8) + pos->y;
    for (entry = ObjInstanceTable; entry < ObjInstanceTable + 128; entry++) {
        if (entry->key == target) {
            return ObjInstanceTable[index].value;
        }
        index++;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0048a010
LEGO_EXPORT void AddInstanceToList(struct InstanceNode *node) {
    struct InstanceList *list;

    list = node->list;
    node->next = 0;
    node->prev = 0;
    if (list->next == 0) {
        list->next = node;
        return;
    }
    node->prev = list->next;
    list->next->next = node;
    node->list->next = node;
}

// FUNCTION: LEGOLAND 0x0048a040
void FreeAllObjectClassInstances(void) {
    struct Ride *node;

    for (node = ObjectClassList; node != 0; node = node->next) {
        void *obj = node->instances;
        while (obj != 0) {
            void *next = *(void **)obj;
            free(obj);
            obj = next;
        }
        node->instances = 0;
    }
}

// FUNCTION: LEGOLAND 0x0048a080
LEGO_EXPORT void RemoveInstanceFromList(struct InstanceNode *node) {
    if (node->next == 0) {
        node->list->next = node->prev;
        if (node->list->next != 0) {
            node->list->next->next = 0;
        }
    } else {
        node->next->prev = node->prev;
    }

    if (node->prev != 0) {
        node->prev->next = node->next;
    }

    node->prev = 0;
    node->next = 0;
}

// FUNCTION: LEGOLAND 0x0048a0c0
LEGO_EXPORT struct ObjInstance *GetInstanceOfClass(struct Ride *ride, const TileId *tile) {
    struct ObjInstance *instance;

    instance = ride->instances;
    if (instance == 0) {
        return 0;
    }
    while (instance->uid != tile->id) {
        instance = instance->next;
        if (instance == 0) {
            return 0;
        }
    }
    return instance;
}

// FUNCTION: LEGOLAND 0x0048a0f0
LEGO_EXPORT void HandleRideAI(void) {}

// FUNCTION: LEGOLAND 0x0048a100
LEGO_EXPORT void RemoveBlokeFromRide(struct Ride *ride, struct RideNode *node) {
    struct Bloke *bloke;
    struct MapElement *element;
    int counter;
    int code;

    bloke = node->rider;
    RemoveBlokeFromList(ride, node);
    if (CountRidersAtTile(ride, &node->tile) == 0) {
        int x = node->tile.pos.x;
        int y = node->tile.pos.y;
        if (x >= 0 && x < lpConfig->width && y >= 0 && y < lpConfig->height) {
            element = GameMap[y] + x;
        } else {
            element = 0;
        }
        element->flags &= 0xfffb;
    }
    free(node);
    bloke->flags &= 0xffdf;
    if (ride->type == 5) {
        if (IsFavouriteFood(bloke, ride->element)) {
            counter = GetBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
            code = CalculateRideCode(bloke->field_7e, bloke->target->data, counter);
            ApplyMoodDelta(bloke, 9, code);
            bloke->field_7c = 0;
        } else {
            counter = GetBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
            code = CalculateRideCode(bloke->field_7e, bloke->target->data, counter);
            ApplyMoodDelta(bloke, 10, code);
            bloke->field_7c = 0;
        }
    } else {
        if (IsFavouriteAttraction(bloke, ride->element)) {
            counter = GetBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
            code = CalculateRideCode(bloke->field_7e, bloke->target->data, counter);
            ApplyMoodDelta(bloke, 0xb, code);
        } else {
            counter = GetBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
            code = CalculateRideCode(bloke->field_7e, bloke->target->data, counter);
            ApplyMoodDelta(bloke, 0xc, code);
        }
    }
    counter = GetBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
    bloke->field_78 = bloke->field_78 + (short)(counter * 0x32);
    bloke->last_ride = bloke->target;
    IncrementBlokeCounter(bloke->target->data, GetBlokeNum(bloke));
    NewLongTermAction(bloke, 0x17);
}

// FUNCTION: LEGOLAND 0x0048a2d0
LEGO_EXPORT void UpdateBlokesOnRide(void) {}

// FUNCTION: LEGOLAND 0x0048a2e0
LEGO_EXPORT void RemoveAllBlokesFromRide(struct Ride *ride, TileId tile) {
    int tx;
    int ty;
    struct RideNode *node;
    struct RideNode *next;
    struct Bloke *bloke;
    struct SampleSource source;

    tx = ride->x + tile.pos.x;
    ty = ride->y + tile.pos.y;
    source.type = 1;
    next = ride->riders;
    while (node = next, node != 0) {
        next = node->next;
        if (node->tile.id == tile.id) {
            bloke = node->rider;
            bloke->person->sprite = NULL;
            bloke->pos.x = tx * 0x100;
            bloke->pos.y = ty * 0x100;
            bloke->height = 0;
            RemoveBlokeFromRide(ride, node);
            BlokeWalkAnim(bloke);
            bloke->flags &= 0xff7f;
            source.bloke = bloke;
            KillAllSamplesFromSource(&source);
        }
    }
}

// FUNCTION: LEGOLAND 0x0048a390
LEGO_EXPORT int GetAllBlokesOffRide(struct Ride *ride, unsigned short uid) {
    struct RideNode *node;
    struct RideNode *next;

    for (node = ride->riders; node != 0; node = next) {
        next = node->next;
        if (node->tile.id == uid) {
            struct Bloke *rider = node->rider;
            if ((rider->flags & 0x40) == 0) {
                rider->flags |= 0x8;
                rider->param_action++;
            }
        }
    }
    return 1;
}

// Finds the TileId of `ride` at map position `pos` (in 1/256 tiles): checks the tiles above, below,
// left and right of it for an object tile (flag 0x80) of this ride whose anchor (ride origin plus
// the tile's field_4/field_5 offset) is the tile itself, and returns that tile's anchor id; 0 if
// none. As in the original, the tile below is only checked when the tile above exists, and it is
// never NULL-checked.
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function and
// each asm block is commented with the C it implements (struct offsets are named in the comments).
// Why asm: the C was at best 77.35%: MSVC6 folds the repeated bounds checks and keeps the map size
// in different registers/slots (the original caches the width in the `pos` slot and the height in
// ebx between the first two checks).
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x0048a3e0
__declspec(naked) LEGO_EXPORT TileId GetObjectUID(struct Point *pos, struct Ride *ride) {
    __asm {
        // tile = *pos >> 8;   (eax = tile.x, edi = tile.y, ebp = ride)
        mov ecx, dword ptr [esp + 4] // pos
        push ebx
        push ebp
        mov ebp, dword ptr [esp + 0x10] // ride
        mov eax, dword ptr [ecx]
        push esi
        push edi
        mov edi, dword ptr [ecx + 4]
        sar eax, 8
        sar edi, 8
        // above: element = on the map ? &GameMap[tile.y - 1][tile.x] : NULL;
        // (caches lpConfig->width in the pos slot [esp+0x14] and lpConfig->height in ebx)
        test eax, eax
        lea ecx, [edi - 1]
        jl left_check
        mov esi, dword ptr [lpConfig]
        xor edx, edx
        mov dx, word ptr [esi + 0x14] // lpConfig->width
        cmp eax, edx
        mov dword ptr [esp + 0x14], edx
        jge left_check_have_config
        test ecx, ecx
        jl left_check_have_config
        xor ebx, ebx
        mov bx, word ptr [esi + 0x16] // lpConfig->height
        cmp ecx, ebx
        jge left_check_have_config
        mov esi, dword ptr [GameMap]
        lea edx, [eax + eax * 4]
        mov ecx, dword ptr [esi + ecx * 4]
        lea ecx, [ecx + edx * 4] // element (20-byte MapElements)
        // if (element == NULL) skip to the left check;
        test ecx, ecx
        je left_check
        // if ((element->flags & 0x80) && element->field_0 && element->field_0->data == ride &&
        //     element->field_4 + ride->x == tile.x && element->field_5 + ride->y == tile.y)
        //     return element->anchor;
        test byte ptr [ecx + 0xc], 0x80 // element->flags
        je below_check
        mov edx, dword ptr [ecx] // element->field_0
        test edx, edx
        je below_check
        cmp dword ptr [edx + 0xc], ebp // ->data == ride
        jne below_check
        mov esi, dword ptr [ebp + 0xc] // ride->x
        xor edx, edx
        mov dl, byte ptr [ecx + 4] // element->field_4
        add esi, edx
        xor edx, edx
        mov dl, byte ptr [ecx + 5] // element->field_5
        add edx, dword ptr [ebp + 0x10] // ride->y
        cmp esi, eax
        jne below_check
        cmp edx, edi
        jne below_check
        mov ax, word ptr [ecx + 4] // element->anchor
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    below_check:
        // below: element = on the map ? &GameMap[tile.y + 1][tile.x] : NULL;  (no NULL check after)
        test eax, eax
        lea ecx, [edi + 1]
        jl below_off_map
        cmp eax, dword ptr [esp + 0x14] // cached width
        jge below_off_map
        test ecx, ecx
        jl below_off_map
        cmp ecx, ebx // cached height
        jge below_off_map
        mov esi, dword ptr [GameMap]
        lea edx, [eax + eax * 4]
        mov ecx, dword ptr [esi + ecx * 4]
        lea esi, [ecx + edx * 4]
        jmp below_test
    below_off_map:
        xor esi, esi
    below_test:
        // same test and return as above, on the tile below
        test byte ptr [esi + 0xc], 0x80
        je left_check
        mov ecx, dword ptr [esi]
        test ecx, ecx
        je left_check
        cmp dword ptr [ecx + 0xc], ebp
        jne left_check
        mov ecx, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [ebp + 0x10]
        xor edx, edx
        mov dl, byte ptr [esi + 4]
        add ecx, edx
        xor edx, edx
        mov dl, byte ptr [esi + 5]
        add edx, ebx
        cmp ecx, eax
        jne left_check
        cmp edx, edi
        jne left_check
        mov ax, word ptr [esi + 4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    left_check:
        mov esi, dword ptr [lpConfig]
    left_check_have_config:
        // left: element = on the map ? &GameMap[tile.y][tile.x - 1] : NULL; same test and return
        lea ecx, [eax - 1]
        test ecx, ecx
        jl right_check
        xor edx, edx
        mov dx, word ptr [esi + 0x14]
        cmp ecx, edx
        jge right_check
        test edi, edi
        jl right_check
        xor edx, edx
        mov dx, word ptr [esi + 0x16]
        cmp edi, edx
        jge right_check
        mov edx, dword ptr [GameMap]
        lea ecx, [ecx + ecx * 4]
        mov edx, dword ptr [edx + edi * 4]
        lea ecx, [edx + ecx * 4]
        test ecx, ecx
        je right_check
        test byte ptr [ecx + 0xc], 0x80
        je right_check
        mov edx, dword ptr [ecx]
        test edx, edx
        je right_check
        cmp dword ptr [edx + 0xc], ebp
        jne right_check
        mov esi, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [ebp + 0x10]
        xor edx, edx
        mov dl, byte ptr [ecx + 4]
        add esi, edx
        xor edx, edx
        mov dl, byte ptr [ecx + 5]
        add edx, ebx
        cmp esi, eax
        jne left_mismatch
        cmp edx, edi
        jne left_mismatch
        mov ax, word ptr [ecx + 4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    left_mismatch:
        mov esi, dword ptr [lpConfig]
    right_check:
        // right: element = on the map ? &GameMap[tile.y][tile.x + 1] : NULL; same test and return
        lea ecx, [eax + 1]
        test ecx, ecx
        jl not_found
        xor edx, edx
        mov dx, word ptr [esi + 0x14]
        cmp ecx, edx
        jge not_found
        test edi, edi
        jl not_found
        xor edx, edx
        mov dx, word ptr [esi + 0x16]
        cmp edi, edx
        jge not_found
        mov edx, dword ptr [GameMap]
        lea ecx, [ecx + ecx * 4]
        mov edx, dword ptr [edx + edi * 4]
        lea ecx, [edx + ecx * 4]
        test ecx, ecx
        je not_found
        test byte ptr [ecx + 0xc], 0x80
        je not_found
        mov edx, dword ptr [ecx]
        test edx, edx
        je not_found
        cmp dword ptr [edx + 0xc], ebp
        jne not_found
        mov esi, dword ptr [ebp + 0xc]
        mov ebx, dword ptr [ebp + 0x10]
        xor edx, edx
        mov dl, byte ptr [ecx + 4]
        add esi, edx
        xor edx, edx
        mov dl, byte ptr [ecx + 5]
        add edx, ebx
        cmp esi, eax
        jne not_found
        cmp edx, edi
        jne not_found
        mov ax, word ptr [ecx + 4]
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    not_found:
        // none.id = 0; return none;
        pop edi
        pop esi
        pop ebp
        xor ax, ax
        pop ebx
        ret
    }
}
// clang-format on
#else
LEGO_EXPORT TileId GetObjectUID(struct Point *pos, struct Ride *ride) {
    struct MapElement *element;
    struct Point tile;
    struct Point probe;
    struct Point origin;
    int n;
    TileId none;

    tile = *pos;
    tile.x >>= 8;
    tile.y >>= 8;
    n = tile.y - 1;
    element = (tile.x >= 0 && tile.x < lpConfig->width && n >= 0 && n < lpConfig->height) ? &GameMap[n][tile.x] : NULL;
    /* the original only looks below when the tile above exists, and never NULL-checks the tile below */
    if (element != NULL) {
        if ((element->flags & 0x80) != 0 && element->field_0 != NULL && element->field_0->data == ride) {
            origin.x = element->field_4 + ride->x;
            origin.y = element->field_5 + ride->y;
            if (origin.x == tile.x && origin.y == tile.y) {
                return element->anchor;
            }
        }
        probe = tile;
        probe.y++;
        element = (probe.x >= 0 && probe.x < lpConfig->width && probe.y >= 0 && probe.y < lpConfig->height) ? &GameMap[probe.y][probe.x] : NULL;
        if ((element->flags & 0x80) != 0 && element->field_0 != NULL && element->field_0->data == ride) {
            origin.x = element->field_4 + ride->x;
            origin.y = element->field_5 + ride->y;
            if (origin.x == tile.x && origin.y == tile.y) {
                return element->anchor;
            }
        }
    }
    probe = tile;
    probe.x--;
    element = (probe.x >= 0 && probe.x < lpConfig->width && probe.y >= 0 && probe.y < lpConfig->height) ? &GameMap[probe.y][probe.x] : NULL;
    if (element != NULL && (element->flags & 0x80) != 0 && element->field_0 != NULL && element->field_0->data == ride) {
        origin.x = element->field_4 + ride->x;
        origin.y = element->field_5 + ride->y;
        if (origin.x == tile.x && origin.y == tile.y) {
            return element->anchor;
        }
    }
    probe.x = tile.x + 1;
    element = (probe.x >= 0 && probe.x < lpConfig->width && probe.y >= 0 && probe.y < lpConfig->height) ? &GameMap[probe.y][probe.x] : NULL;
    if (element != NULL && (element->flags & 0x80) != 0 && element->field_0 != NULL && element->field_0->data == ride) {
        origin.x = element->field_4 + ride->x;
        origin.y = element->field_5 + ride->y;
        if (origin.x == tile.x && origin.y == tile.y) {
            return element->anchor;
        }
    }
    none.id = 0;
    return none;
}
#endif
