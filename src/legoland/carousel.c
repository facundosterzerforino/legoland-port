#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "binv.h"
#include "bloke.h"
#include "carousel.h"
#include "gamemap.h"
#include "image_sprite.h"
#include "llidb.h"
#include "man3d.h"
#include "map_object.h"
#include "math.h"
#include "obj_instance.h"
#include "print_sprite.h"
#include "render3d.h"
#include "sound_music.h"
#include "tilemap.h"

// FUNCTION: LEGOLAND 0x0042bbc0
void AddCarouselNode(unsigned short *param_1) {
    struct CarouselNode *node = (struct CarouselNode *)malloc(sizeof(struct CarouselNode));
    if (node != NULL) {
        unsigned int *fill = (unsigned int *)node;
        int i;
        for (i = 0xb; i != 0; i--) {
            *fill = 0;
            fill++;
        }
        node->id = *param_1;
        node->next = CarouselNodeList;
        CarouselNodeList = node;
        FUN_0042c210(node);
    }
}

// FUNCTION: LEGOLAND 0x0042bc00
void RemoveCarouselNode(struct CarouselNode *node) {
    struct CarouselNode *cur;
    struct CarouselNode *prev;

    if (CarouselNodeList == node) {
        CarouselNodeList = node->next;
    } else {
        cur = CarouselNodeList->next;
        prev = CarouselNodeList;
        while (cur != node) {
            prev = prev->next;
            if (prev == NULL) {
                break;
            }
            cur = prev->next;
        }
        if (prev != NULL) {
            prev->next = node->next;
        }
    }
    free(node);
}

// FUNCTION: LEGOLAND 0x0042bc40
void FreeAllCarouselNodes(void) {
    while (CarouselNodeList != NULL) {
        RemoveCarouselNode(CarouselNodeList);
    }
}

// FUNCTION: LEGOLAND 0x0042bc60
struct CarouselNode *FindCarouselNode(unsigned short *param_1) {
    struct CarouselNode *node;

    node = CarouselNodeList;
    if (node == NULL) {
        return NULL;
    }
    while (memcmp(&node->id, param_1, sizeof(node->id)) != 0) {
        node = node->next;
        if (node == NULL) {
            return NULL;
        }
    }
    return node;
}

// FUNCTION: LEGOLAND 0x0042bc90
void FUN_0042bc90(struct CarouselNode *node) {
    struct SampleParams params;

    params.field_0 = 2;
    node->leaving_count = node->seated_count;
    node->flags = node->flags & 0xffffbfff | 1;
    node->seated_count = 0;
    node->frame_ticks = 0;
    params.x = *(unsigned char *)((char *)node + 4);
    params.y = *(unsigned char *)((char *)node + 5);
    node->frame = 1;
    PlayInstanceOfSample(*(void **)(CAROUSSEL_SFX + 8), 1, 1, &params);
}

// FUNCTION: LEGOLAND 0x0042c210
void FUN_0042c210(struct CarouselNode *node) {
    int r;
    struct SampleParams params;

    node->frame_ticks = 0;
    node->frame = 0;
    r = rand() % 2;
    node->boarding_count = 0;
    node->cycles_left = (char)r + '\x03';
    node->seated_count = 0;
    node->flags = node->flags & 0xffffbffe;
    params.field_0 = 2;
    params.x = *(unsigned char *)((char *)node + 4);
    params.y = *(unsigned char *)((char *)node + 5);
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x0042c280
void LoadCarouselResources(struct CarouselRideObj *param_1) {
    struct LayerResult layer;

    DAT_006160bc = param_1->ride;
    Load_FXList(CAROUSSEL_SFX, 2);
    DAT_006160bc->flags |= 0x420;
    CarouselLayer = DAT_006160bc->layer;
    *(unsigned int *)((char *)CarouselLayer + 0x10) |= 0x2000;
    GetLayer((struct LayerOwner *)DAT_006160bc->layer, &layer, 0);
    DAT_00616078 = layer.x + -0x58;
    DAT_0061607c = layer.y + -0xcd;
    // STRING: LEGOLAND 0x004b65a4
    ZCarouselSprite = LoadSprite("z_Carousel.lls", 1);
    DAT_006160c0 = ZCarouselSprite;
    // STRING: LEGOLAND 0x004b658c
    CarouselOnBinV = LoadBinV("Zbuffers\\CarouselOn.bnv");
    DAT_00616090 = CarouselOnBinV;
    // STRING: LEGOLAND 0x004b6574
    CarouselBinV = LoadBinV("Zbuffers\\Carousel.bnv");
    DAT_00616094 = CarouselBinV;
    // STRING: LEGOLAND 0x004b6558
    CarouselOffBinV = LoadBinV("Zbuffers\\CarouselOff.bnv");
    DAT_00616098 = CarouselOffBinV;
    // STRING: LEGOLAND 0x004b653c
    CarouselEntranceMatteSprite = LoadSprite("Carousel Entrance Matte.lls", 1);
    // STRING: LEGOLAND 0x004b651c
    CarouselEntranceMatte2Sprite = LoadSprite("Carousel Entrance Matte2.lls", 1);
    HideLayer(CarouselLayer, 2);
    StopLayerPlaying((unsigned int)CarouselLayer, 2);
    LLSSetFrame((struct LLS *)GetLLSForLayer((unsigned int)CarouselLayer, 2), 0);
    HideLayer(CarouselLayer, 0);
    StopLayerPlaying((unsigned int)CarouselLayer, 0);
    LLSSetFrame((struct LLS *)GetLLSForLayer((unsigned int)CarouselLayer, 0), 0);
    HideLayer(CarouselLayer, 1);
}

// FUNCTION: LEGOLAND 0x0042c3f0
void UnloadCarouselResources(struct CarouselRideObj *input) {
    DAT_006160bc = input->ride;
    KillSprite(CarouselEntranceMatteSprite);
    KillSprite(CarouselEntranceMatte2Sprite);
    KillSprite(DAT_006160c0);
    FreeAllCarouselNodes();
    Kill_FXList(CAROUSSEL_SFX, 2);
    FreeBinV(DAT_00616090);
    FreeBinV(DAT_00616094);
    FreeBinV(DAT_00616098);
}

// FUNCTION: LEGOLAND 0x0042c460
void CarouselSetEditMode(void) {
    EditMode.unk0 = 1;
    EditMode.unk8 = DAT_006160bc;
    DefaultCursor(&EditCursor);
    SetEditCursorFootPrint((char *)EditMode.unk8 + 0x3c);
}

// FUNCTION: LEGOLAND 0x0042c4a0
void CarouselRemoveObject(struct CarouselRideObj *param_1, TileId tile, unsigned int param_3) {
    struct CarouselNode *node;
    struct SampleParams params;

    node = FindCarouselNode(&tile.id);
    if (node != NULL) {
        RemoveCarouselNode(node);
    }
    StandardRemoveObject((unsigned int)param_1, tile, param_3);
    RemoveAllBlokesFromRide(param_1->ride, tile);
    params.x = tile.pos.x;
    params.y = tile.pos.y;
    params.field_0 = 2;
    UnSourceAndFadeAllSamplesFromSource(&params, 0xffffff38);
}

// FUNCTION: LEGOLAND 0x0042c520
void CarouselAddObject(unsigned int param_1, unsigned char *param_2) {
    unsigned char pair[2];

    pair[0] = param_2[0];
    pair[1] = param_2[4];
    AddBasicObject(param_1, (unsigned int)param_2);
    AddCarouselNode((unsigned short *)pair);
}

// FUNCTION: LEGOLAND 0x0042c550
struct RideSpriteInfo *GetCarouselSpriteInfo(struct CarouselRideObj *param1, unsigned short param2) {
    struct CarouselRide *ride = param1->ride;

    DAT_006160a0.sprite = ride->layer;
    DAT_006160a0.x = *(unsigned int *)((char *)ride + 0x14);
    DAT_006160a0.y = *(unsigned int *)((char *)ride + 0x18);
    DAT_006160a0.id = param2;
    *(unsigned int *)((char *)ride->layer + 0x10) |= 0x2000;
    return &DAT_006160a0;
}

// FUNCTION: LEGOLAND 0x0042c590
int Carousel_Save(void) {
    struct CarouselNode *current = CarouselNodeList;
    unsigned int flag = 1;
    unsigned int terminator = 0;

    while (current != NULL) {
        if (!SaveGameWrite(&flag, 4)) {
            return 0;
        }
        if (!SaveGameWrite(current, 0x2c)) {
            return 0;
        }
        current = current->next;
    }

    if (SaveGameWrite(&terminator, 4)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0042c600
int Carousel_Load(struct CarouselRideObj *param_1) {
    struct CarouselRide *ride = param_1->ride;
    struct CarouselNode *node;
    struct CarouselNode *prev;
    struct CarouselListElem *elem;
    unsigned int marker;

    prev = NULL;
    if (SaveGameRead(&marker, 4) == 0) {
        return 0;
    }
    while (marker != 0) {
        node = (struct CarouselNode *)malloc(sizeof(struct CarouselNode));
        if (SaveGameRead(node, 0x2c) == 0) {
            return 0;
        }
        node->next = NULL;
        if (prev != NULL) {
            prev->next = node;
        } else {
            CarouselNodeList = node;
        }
        prev = node;
        if (SaveGameRead(&marker, 4) == 0) {
            return 0;
        }
    }
    for (elem = ride->list; elem != NULL; elem = elem->next) {
        unsigned int *comp = *(unsigned int **)((char *)elem + 0x10);
        if (comp[0xc] != 0) {
            comp[0xb] = ((unsigned int *)&DAT_006160bc)[comp[0xc]];
        } else {
            comp[0xb] = 0;
            (*(unsigned int **)((char *)elem + 0x10))[0xc] = 0;
        }
        {
            unsigned int *h = *(unsigned int **)((char *)elem->bloke + 0x54);
            if (h != NULL) {
                *h = ((unsigned int *)&DAT_00616090)[h[1]];
            }
        }
    }
    return 1;
}

// FUNCTION: LEGOLAND 0x0042c6d0
void FUN_0042c6d0(struct CarouselNode *node) {
    struct CarouselListElem *elem = DAT_006160bc->list;
    unsigned int flags = node->flags;

    if ((flags & 1) != 0) {
        int v;
        unsigned char f10;
        v = node->frame_ticks + 1;
        node->frame_ticks = v;
        f10 = node->cycles_left;
        v = node->frame_ticks;
        if (f10 == 0) {
            if (GetAllBlokesOffRide((struct Ride *)DAT_006160bc, node->id) == 0) {
                return;
            }
            FUN_0042c210(node);
            return;
        }
        if (2 <= v) {
            char cVar4;
            node->frame_ticks = 0;
            cVar4 = ++node->frame;
            if (cVar4 >= '@') {
                node->frame = 0;
                node->cycles_left = f10 - 1;
            }
        }
    } else if ((flags & 0x4000) != 0) {
        if ((char)node->seated_count == (char)node->boarding_count) {
            node->flags = flags & 0xffffbfff;
            FUN_0042bc90(node);
            return;
        }
    } else if (node->seated_count != 0) {
        if (node->boarding_timer == 0) {
            node->flags = flags | 0x4000;
            Ride_SetFlagToNotLetAnyoneOn((unsigned char *)&node->id);
        } else {
            node->boarding_timer = node->boarding_timer - 1;
        }
    }
    for (; elem != NULL; elem = elem->next) {
        if (node->id == elem->id && *(char *)((char *)elem->bloke + 0x35) == '\x01') {
            // STRING: LEGOLAND 0x004b4704
            sprintf(DAT_004b64d4, "%02d", *(unsigned char *)((char *)elem->bloke + 0x36));
            // STRING: LEGOLAND 0x004b64cc
            SetBlokePositionFromBNV(CarouselBinV, elem->bloke, "BlokeBox??", (int)(char)node->frame, -1617853.25f, -1618109.0f, 0);
        }
    }
    *(short *)**(int **)((char *)ZCarouselSprite + 8) = (short)(char)node->frame;
}

// FUNCTION: LEGOLAND 0x0042c800
void FUN_0042c800(void) {
    struct CarouselNode *current = CarouselNodeList;

    while (current != NULL) {
        FUN_0042c6d0(current);
        current = current->next;
    }
}

// FUNCTION: LEGOLAND 0x0042c820
void FUN_0042c820(struct CarouselRideObj *param_1) {
    struct CarouselRide *ride = param_1->ride;
    struct CarouselListElem *elem = ride->list;
    struct CarouselListElem *next;
    int bloke;
    int blokepos;
    unsigned char *pos;
    int iVar12, iVar13;
    char cVar7;
    int local_30, local_2c;
    int local_18, local_14;
    int local_c, local_8;
    struct Point coords;

    while (elem != NULL) {
        next = elem->next;
        blokepos = (int)elem->bloke;
        pos = (unsigned char *)&elem->id;
        bloke = (int)FindCarouselNode((unsigned short *)pos);
        if (bloke == 0) {
            break;
        }
        iVar12 = *(int *)((char *)ride + 0xc) + (unsigned int)*pos;
        iVar13 = (unsigned int)pos[1] + *(int *)((char *)ride + 0x10);
        if (*(short *)(blokepos + 0xe) == 0) {
            switch (*(unsigned char *)(blokepos + 0x60)) {
            case 0:
                iVar13 = iVar13 * 0x100 + 0x80;
                *(char *)(bloke + 0x18) = *(char *)(bloke + 0x18) + '\x01';
                iVar12 = (iVar12 + -3) * 0x100;
                *(unsigned int *)(blokepos + 0x1c) = 0xb4;
                *(unsigned char *)(blokepos + 0x62) |= 8;
                *(int *)(blokepos + 0x24) = iVar12;
                *(int *)(blokepos + 0x28) = iVar13;
                cVar7 = CalcMoveLine(*(struct Point *)(blokepos + 0x68), *(struct Point *)(blokepos + 0x24), (struct Navigator *)(blokepos + 0x98));
                *(short *)(blokepos + 0xe) = 7;
                *(unsigned char *)(blokepos + 0x73) = cVar7 + 0x10;
                NewDirForAction(blokepos, ((unsigned char)(cVar7 + 0x10) >> 5) + 3);
                *(unsigned int *)(blokepos + 0x58) = 0;
                *(char *)(blokepos + 0x60) = *(char *)(blokepos + 0x60) + '\x01';
                break;
            case 1:
                coords = GetScreenCoordsForObject(pos, ride);
                {
                    int iVar10 = *(int *)(blokepos + 0x6c);
                    int iVar10b = *(int *)(blokepos + 0x68);
                    short sVar8, sVar9;
                    GetTileDimensions(&local_30, &local_2c);
                    iVar13 = (iVar10b + iVar10) * local_2c;
                    iVar12 = (iVar10b - iVar10) * local_30;
                    sVar8 = Get_XScroll();
                    sVar9 = Get_YScroll();
                    local_18 = (((((unsigned int)lpConfig->view_x - (int)sVar8) + (iVar12 >> 9)) - DAT_00616078 / 2) - coords.x) * 2;
                    local_14 = ((((iVar13 >> 9) + ((unsigned int)lpConfig->view_y - (int)sVar9)) - DAT_0061607c / 2) - coords.y) * 2;
                }
                *(struct Sprite **)(*(int *)(blokepos + 4) + 0x2c) = DAT_006160c0;
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x30) = 1;
                *(float *)(*(int *)(blokepos + 4) + 0x3c) = GetUnitDepth(-1617853.25f, -1618109.0f);
                *(unsigned char *)(blokepos + 0x35) = 0;
                sprintf(DAT_004b64d4, "%02d", FUN_0042cd20(elem, (struct CarouselNode *)bloke, *(unsigned char *)((char *)DAT_006160bc + 0x2e)));
                *(unsigned int *)(blokepos + 0x54) = (unsigned int)NewBNVPath(DAT_00616090, 0, "BlokeBox??", -1617853.25f, -1618109.0f, &local_18);
                UpdateBlokeFromBNVPath(blokepos, *(unsigned int *)(blokepos + 0x54));
                *(unsigned char *)(blokepos + 0x62) |= 0x80;
                *(char *)(blokepos + 0x60) = *(char *)(blokepos + 0x60) + '\x01';
                break;
            case 2:
                if (UpdateBlokeFromBNVPath(blokepos, *(unsigned int *)(blokepos + 0x54)) == 0) {
                    *(unsigned char *)(blokepos + 0x35) = 1;
                    *(unsigned char *)(blokepos + 0x60) = 5;
                    free(*(void **)(blokepos + 0x54));
                    *(unsigned int *)(blokepos + 0x54) = 0;
                }
                BlokeSetFrame(blokepos, *(unsigned char *)(blokepos + 0x74));
                break;
            case 5:
                *(unsigned char *)(blokepos + 0x62) |= 0x80;
                BlokeSetFrame(blokepos, 0);
                *(unsigned char *)(blokepos + 0x35) = 1;
                *(struct Sprite **)(*(int *)(blokepos + 4) + 0x2c) = DAT_006160c0;
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x30) = 1;
                *(float *)(*(int *)(blokepos + 4) + 0x3c) = GetUnitDepth(-1617853.25f, -1618109.0f);
                *(char *)(blokepos + 0x60) = *(char *)(blokepos + 0x60) + '\x01';
                cVar7 = *(char *)(bloke + 6) + '\x01';
                *(char *)(bloke + 6) = cVar7;
                if ((short)cVar7 == *(short *)((char *)DAT_006160bc + 0x2e)) {
                    FUN_0042bc90((struct CarouselNode *)bloke);
                }
                break;
            case 7:
                local_c = (int)*(short *)(blokepos + 0x3c) << 1;
                local_8 = (int)*(short *)(blokepos + 0x3e) << 1;
                BlokeWalkAnim((struct Bloke *)blokepos);
                BlokeSetFrame(blokepos, 0);
                *(unsigned char *)(blokepos + 0x62) |= 0x80;
                *(struct Sprite **)(*(int *)(blokepos + 4) + 0x2c) = DAT_006160c0;
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x30) = 1;
                *(float *)(*(int *)(blokepos + 4) + 0x3c) = GetUnitDepth(-1617853.25f, -1618109.0f);
                *(unsigned char *)(blokepos + 0x35) = 2;
                sprintf(DAT_004b64d4, "%02d", *(unsigned char *)(blokepos + 0x36));
                *(unsigned int *)(blokepos + 0x54) = (unsigned int)NewBNVPath(DAT_00616098, 2, "BlokeBox??", -1617853.25f, -1618109.0f, &local_c);
                *(char *)(blokepos + 0x60) = *(char *)(blokepos + 0x60) + '\x01';
                break;
            case 8:
                if (UpdateBlokeFromBNVPath(blokepos, *(unsigned int *)(blokepos + 0x54)) == 0) {
                    *(unsigned char *)(blokepos + 0x35) = 2;
                    *(unsigned char *)(blokepos + 0x60) = 0xd;
                    free(*(void **)(blokepos + 0x54));
                    *(unsigned int *)(blokepos + 0x54) = 0;
                    *(unsigned char *)(blokepos + 0x72) = 3;
                }
                BlokeSetFrame(blokepos, *(unsigned char *)(blokepos + 0x74));
                break;
            case 0xd:
                *(unsigned char *)(*(unsigned char *)(blokepos + 0x36) + 0x1f + bloke) = 0;
                *(unsigned short *)(blokepos + 0x62) &= 0xff7f;
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x2c) = 0;
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x30) = 0;
                UnAdjustBlokePosition(*(int *)(blokepos + 4) + 0x1c);
                ScreenToMapRef(*(int *)(blokepos + 4) + 0x1c, blokepos + 0x68, 0);
                *(unsigned int *)(*(int *)(blokepos + 4) + 0x34) = 0;
                iVar13 = iVar13 * 0x100 + 0x80;
                *(int *)(blokepos + 0x6c) = *(int *)(blokepos + 0x6c) << 8;
                *(int *)(blokepos + 0x68) = *(int *)(blokepos + 0x68) << 8;
                iVar12 = iVar12 * 0x100 + 0x80;
                *(int *)(blokepos + 0x24) = iVar12;
                *(int *)(blokepos + 0x28) = iVar13;
                cVar7 = CalcMoveLine(*(struct Point *)(blokepos + 0x68), *(struct Point *)(blokepos + 0x24), (struct Navigator *)(blokepos + 0x98));
                *(short *)(blokepos + 0xe) = 7;
                *(unsigned char *)(blokepos + 0x73) = cVar7 + 0x10;
                NewDirForAction(blokepos, ((unsigned char)(cVar7 + 0x10) >> 5) + 3);
                *(char *)(blokepos + 0x60) = *(char *)(blokepos + 0x60) + '\x01';
                break;
            case 0xe:
                RemoveBlokeFromRide((void *)((char *)ride), elem);
                *(unsigned short *)(blokepos + 0x62) &= 0xfff7;
                cVar7 = *(char *)(bloke + 7) + -1;
                *(char *)(bloke + 7) = cVar7;
                if (cVar7 == '\0') {
                    *(unsigned char *)(bloke + 6) = 0;
                    Ride_ClearFlagToNotLetAnyoneOn((unsigned char *)(bloke + 4));
                }
                break;
            }
        }
        elem = next;
    }
    FUN_0042c800();
}

// FUNCTION: LEGOLAND 0x0042bcf0
void RenderCarousel(struct CarouselRideObj *param_1, unsigned int param_2, unsigned int param_3, unsigned short *param_4, unsigned int param_5, unsigned int param_6) {
    struct CarouselRide *ride = param_1->ride;
    struct CarouselListElem *elem;
    int bloke;
    int iVar6, iVar10;
    char cVar5;
    unsigned int uVar7;
    struct LayerResult layerres;
    int local_68;
    struct Point local_5c, local_54;
    unsigned int local_4c;
    int local_48;
    short local_44;
    int local_28[11];

    iVar6 = (int)ride;
    elem = ride->list;
    {
        int *fill = local_28;
        int n;
        local_28[0] = 0;
        for (n = 9; fill = fill + 1, n != 0; n--) {
            *fill = 0;
        }
    }
    local_44 = *param_4;
    cVar5 = '\0';
    local_4c = 0x103;
    local_48 = (int)param_1;
    bloke = (int)FindCarouselNode(param_4);
    if (bloke != 0) {
        struct Point sc = GetScreenCoordsForObject((unsigned char *)param_4, ride);
        iVar10 = sc.y;
        iVar6 = sc.x;
        GetLayer((struct LayerOwner *)ride->layer, &layerres, 0);
        if (elem != NULL) {
            short sVar1 = *param_4;
            do {
                if (sVar1 == (short)elem->id) {
                    int idx = (int)cVar5;
                    cVar5 = cVar5 + '\x01';
                    local_28[idx] = (int)elem->bloke;
                }
                elem = elem->next;
            } while (elem != NULL);
            if (cVar5 != '\0') {
                uVar7 = GetLLSForLayer((unsigned int)CarouselLayer, 0);
                LLSSetFrame((struct LLS *)uVar7, (int)*(char *)(bloke + 8));
                local_5c = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 0);
                AdjustOffsetForViewMode(&local_5c);
                uVar7 = GetSpriteForLayer((struct LayerContainer *)CarouselLayer, 0);
                PrintSprite((struct Sprite *)uVar7, local_5c.x + iVar6, local_5c.y + iVar10, param_6, (int *)&local_4c);
                local_54 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 1);
                AdjustOffsetForViewMode(&local_54);
                PrintSprite(CarouselEntranceMatte2Sprite, local_54.x + iVar6, local_54.y + iVar10, param_6, (int *)&local_4c);
                uVar7 = GetLLSForLayer((unsigned int)CarouselLayer, 2);
                LLSSetFrame((struct LLS *)uVar7, (int)*(char *)(bloke + 8));
                local_5c = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 2);
                AdjustOffsetForViewMode(&local_5c);
                uVar7 = GetSpriteForLayer((struct LayerContainer *)CarouselLayer, 2);
                PrintSprite((struct Sprite *)uVar7, local_5c.x + iVar6, local_5c.y + iVar10, param_6, (int *)&local_4c);
                if ('\0' < cVar5) {
                    int *p;
                    local_68 = (int)cVar5;
                    p = local_28;
                    do {
                        if (*(char *)(*p + 0x60) == '\0') {
                            IP_RenderBlokeIn3DNow((struct Bloke *)*p);
                        }
                        p = p + 1;
                        local_68 = local_68 + -1;
                    } while (local_68 != 0);
                    if ('\0' < cVar5) {
                        local_68 = (int)cVar5;
                        p = local_28;
                        do {
                            if (*(char *)(*p + 0x60) == '\x01') {
                                IP_RenderBlokeIn3DNow((struct Bloke *)*p);
                            }
                            p = p + 1;
                            local_68 = local_68 + -1;
                        } while (local_68 != 0);
                        if ('\0' < cVar5) {
                            local_68 = (int)cVar5;
                            p = local_28;
                            do {
                                if (*(char *)(*p + 0x60) == '\r') {
                                    IP_RenderBlokeIn3DNow((struct Bloke *)*p);
                                }
                                p = p + 1;
                                local_68 = local_68 + -1;
                            } while (local_68 != 0);
                            if ('\0' < cVar5) {
                                int param1c = (int)cVar5;
                                p = local_28;
                                do {
                                    if (*(char *)(*p + 0x60) == '\x0e') {
                                        IP_RenderBlokeIn3DNow((struct Bloke *)*p);
                                    }
                                    p = p + 1;
                                    param1c = param1c + -1;
                                } while (param1c != 0);
                            }
                        }
                    }
                }
                *(short *)**(int **)((char *)ZCarouselSprite + 8) = (short)*(char *)(bloke + 8);
                {
                    struct CarouselListElem *e;
                    int local_64, local_60;
                    int iVar3 = DAT_00616078;
                    int iVar4 = DAT_0061607c;
                    for (e = ride->list; local_60 = iVar4, local_64 = iVar3, DAT_00616078 = local_64,
                        DAT_0061607c = local_60, e != NULL;
                        e = e->next) {
                        int b = (int)e->bloke;
                        if (*param_4 == (short)e->id && (*(unsigned char *)(b + 0x62) & 0x80) != 0) {
                            int unit = *(int *)(b + 4);
                            *(int *)(unit + 0x24) = (int)*(short *)(b + 0x3c);
                            *(int *)(unit + 0x28) = (int)*(short *)(b + 0x3e);
                            AdjustBlokePosition((struct Point *)(unit + 0x24));
                            AdjustOffsetForViewMode((struct Point *)&local_64);
                            *(int *)(unit + 0x1c) = *(short *)(b + 0x3c) + local_64 + iVar6;
                            *(int *)(unit + 0x20) = *(short *)(b + 0x3e) + local_60 + iVar10;
                            AdjustBlokePosition((struct Point *)(unit + 0x1c));
                            IP_RenderBlokeIn3DNow(e->bloke);
                        }
                        iVar3 = DAT_00616078;
                        iVar4 = DAT_0061607c;
                    }
                }
                local_54 = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 1);
                AdjustOffsetForViewMode(&local_54);
                PrintSprite(CarouselEntranceMatteSprite, local_54.x + iVar6, local_54.y + iVar10, param_6, 0);
                return;
            }
        }
        uVar7 = GetLLSForLayer((unsigned int)CarouselLayer, 0);
        LLSSetFrame((struct LLS *)uVar7, (int)*(char *)(bloke + 8));
        local_5c = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 0);
        AdjustOffsetForViewMode(&local_5c);
        uVar7 = GetSpriteForLayer((struct LayerContainer *)CarouselLayer, 0);
        PrintSprite((struct Sprite *)uVar7, local_5c.x + iVar6, local_5c.y + iVar10, param_6, (int *)&local_4c);
        local_5c = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 1);
        AdjustOffsetForViewMode(&local_5c);
        uVar7 = GetSpriteForLayer((struct LayerContainer *)CarouselLayer, 1);
        PrintSprite((struct Sprite *)uVar7, local_5c.x + iVar6, local_5c.y + iVar10, param_6, (int *)&local_4c);
        uVar7 = GetLLSForLayer((unsigned int)CarouselLayer, 2);
        LLSSetFrame((struct LLS *)uVar7, (int)*(char *)(bloke + 8));
        local_5c = GetRenderOffsetForLayer((struct LayerOffsetHolder *)CarouselLayer, 2);
        AdjustOffsetForViewMode(&local_5c);
        uVar7 = GetSpriteForLayer((struct LayerContainer *)CarouselLayer, 2);
        PrintSprite((struct Sprite *)uVar7, local_5c.x + iVar6, local_5c.y + iVar10, param_6, (int *)&local_4c);
    }
}

// FUNCTION: LEGOLAND 0x0042cd20
int FUN_0042cd20(struct CarouselListElem *elem, struct CarouselNode *node, signed char divisor) {
    int count = divisor;
    int eax = rand();
    int index = eax % count;
    unsigned char *slots = (unsigned char *)node;

    while (slots[0x20 + index] != 0) {
        index++;
        if (index >= count) {
            index = 0;
        }
    }

    slots[0x20 + index] = 1;
    *(char *)((char *)elem->bloke + 0x36) = (char)(index + 1);
    return index + 1;
}
