#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "legoland.h"

#include "bloke.h"
#include "build.h"
#include "challenge.h"
#include "controller.h"
#include "draw.h"
#include "gamemain.h"
#include "gamemap.h"
#include "interface.h"
#include "llidb.h"
#include "map_object.h"
#include "nerps.h"
#include "objclass.h"
#include "objectives.h"
#include "path_control.h"
#include "pathfind.h"
#include "resource.h"
#include "string.h"
#include "tilemap.h"
#include "title.h"
#include "worker.h"

struct GameMainNode {
    struct GameMainNode *next;
    struct GameMainNode *parent;
    int x;
    int y;
    int step_cost;
    int path_cost;
    int heuristic;
    int field_1c;
    int field_20;
    int field_24;
};

struct GameMainArg {
    unsigned int field_0;
    unsigned int field_4;
};

struct GameMainEntry {
    unsigned char pad_0[0x4];
    char *name;
};

struct EventNode {
    struct EventNode *next;
    unsigned char pad_4[0x1c - 0x4];
    int sort_key;
};

struct QueryNode {
    struct QueryNode *next;
    unsigned char pad_4[0x8 - 0x4];
    unsigned int field_8;
    unsigned int field_c;
};

// FUNCTION: LEGOLAND 0x00477680
int IsInMapBounds(int a, int b) {
    unsigned short limit;

    if (a < 0) {
        return 0;
    }
    if (b < 0) {
        return 0;
    }
    if (a >= lpConfig->width) {
        return 0;
    }
    limit = lpConfig->height;
    return b < limit;
}

// FUNCTION: LEGOLAND 0x004776c0
void FUN_004776c0(struct QueryNode *node) {
    node->next = (struct QueryNode *)DAT_00668fc4;
    DAT_00668fc4 = (struct InterfaceQueryNode *)node;
}

// FUNCTION: LEGOLAND 0x004776e0
void InsertOpenListSorted(struct EventNode *node) {
    struct EventNode *current;
    struct EventNode *previous;

    if (DAT_00668fc0 == NULL) {
        DAT_00668fc0 = node;
        node->next = NULL;
    } else {
        current = DAT_00668fc0;
        previous = NULL;
        while (current != NULL) {
            if (current->sort_key >= node->sort_key) {
                break;
            }
            previous = current;
            current = current->next;
        }
        if (previous != NULL) {
            node->next = previous->next;
            previous->next = node;
        } else {
            node->next = DAT_00668fc0;
            DAT_00668fc0 = node;
        }
    }
}

// FUNCTION: LEGOLAND 0x00477730
struct GameMainNode *FUN_00477730(struct Point *ctx) {
    struct GameMainNode *node;

    node = (struct GameMainNode *)DAT_00668fc4;
    if (node == NULL) {
        return NULL;
    }
    while (node != NULL) {
        if (node->x == ctx->x && node->y == ctx->y) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// FUNCTION: LEGOLAND 0x00477760
void RemoveQueryNode(struct QueryNode *ctx) {
    struct QueryNode *prev;
    struct QueryNode *node;

    prev = NULL;
    node = (struct QueryNode *)DAT_00668fc4;
    while (node != NULL) {
        if (node == ctx) {
            break;
        }
        prev = node;
        node = node->next;
    }
    if (prev != NULL) {
        prev->next = node->next;
    } else {
        DAT_00668fc4 = (struct InterfaceQueryNode *)node->next;
    }
}

// FUNCTION: LEGOLAND 0x00477790
void RemoveFromOpenList(struct EventNode *param_1) {
    struct EventNode *prev;
    struct EventNode *node;

    prev = NULL;
    node = DAT_00668fc0;
    while (node != NULL) {
        if (node == param_1) {
            break;
        }
        prev = node;
        node = node->next;
    }
    if (prev != NULL) {
        prev->next = node->next;
        return;
    }
    DAT_00668fc0 = node->next;
}

// FUNCTION: LEGOLAND 0x004777c0
struct GameMainNode *FUN_004777c0(struct Point *arg) {
    struct GameMainNode *node = DAT_00668fc0;
    int first;

    if (node == NULL) {
        return NULL;
    }
    first = arg->x;
    do {
        if (node->x == first && node->y == arg->y) {
            return node;
        }
        node = node->next;
    } while (node != NULL);
    return NULL;
}

// FUNCTION: LEGOLAND 0x004777f0
struct GameMainNode *FUN_004777f0(struct Point *pos, int *result) {
    struct GameMainNode *node;
    struct MapElement *tile;
    struct Ride *ride;
    unsigned short flags;
    int dx;
    int dy;

    node = FUN_00477730(pos);
    if (node != NULL) {
        *result = 2;
        return node;
    }
    node = FUN_004777c0(pos);
    if (node != NULL) {
        *result = 1;
        return node;
    }
    *result = 0;
    node = (struct GameMainNode *)malloc(0x28);
    if (pos->x >= 0 && pos->x < lpConfig->width && pos->y >= 0 && pos->y < lpConfig->height) {
        tile = &GameMap[pos->y][pos->x];
    } else {
        tile = NULL;
    }
    flags = tile->flags;
    if ((flags & 0x10) || (tile->field_10 & 1)) {
        if (FUN_00482b60(pos)) {
            node->field_20 = 1;
        } else {
            node->field_20 = 0;
        }
        node->step_cost = 1;
    } else if (flags & 0x40) {
        node->field_20 = 5;
        node->step_cost = -1;
    } else if (flags & 0x8a0) {
        ride = tile->field_0->ride;
        if (ride->flags & 0x200000) {
            if (ride->range > 1) {
                node->field_20 = 4;
                node->step_cost = 0x14;
            } else {
                node->field_20 = 3;
                node->step_cost = 9;
            }
        } else {
            node->field_20 = 5;
            node->step_cost = -1;
        }
    } else if (tile->field_10 & 2) {
        node->field_20 = 5;
        node->step_cost = -1;
    } else {
        node->field_20 = 2;
        node->step_cost = 3;
    }
    node->x = pos->x;
    node->y = pos->y;
    node->path_cost = 0x7fffffff;
    dy = abs(pos->y - DAT_004bb5a4);
    dx = abs(pos->x - DAT_004bb5a0);
    node->parent = 0;
    node->field_24 = 0;
    node->heuristic = dx + dy;
    node->field_1c = node->step_cost + node->heuristic;
    return node;
}

// FUNCTION: LEGOLAND 0x00477980
unsigned int FUN_00477980(unsigned int param_1, unsigned int param_2) {
    return ((param_1 & param_2) != 0 ? 0xfffffffc : 0u) + 4;
}

// FUNCTION: LEGOLAND 0x004779a0
int FUN_004779a0(int x0, int y0, int x1, int y1) {
    int dx = x0 - x1;
    int dy = y0 - y1;

    if (dx != 0 && dy != 0) {
        return 0;
    }
    if (dx != 0) {
        return 2;
    }
    return dy != 0;
}

// FUNCTION: LEGOLAND 0x004779d0
void FUN_004779d0(struct Point *p) {
    struct MapElement *tile;
    struct Element *elem;
    struct ObjClass *saved_class;
    struct Cursor saved;
    TileId t;
    struct Point pos;
    WorkOrder *order;

    if (p->x >= 0 && p->x < lpConfig->width && p->y >= 0 && p->y < lpConfig->height) {
        tile = &GameMap[p->y][p->x];
    } else {
        tile = NULL;
    }
    if (tile->flags & 0x40) {
        return;
    }
    if (tile->flags & 0xa0) {
        elem = tile->field_0;
        saved_class = QueryClass;
        memcpy(&saved, &QueryCursor, sizeof(struct Cursor));
        QueryClass = (struct ObjClass *)elem->data;
        t.pos.x = tile->field_4;
        pos.x = t.pos.x;
        t.pos.y = tile->field_5;
        pos.y = t.pos.y;
        QueryClass->method_94((unsigned int *)elem, &pos);
        if (tile->flags & 0x20) {
            if (QueryClass->flags & 0x200000) {
                RemoveObjectFromBuildList(t);
                FUN_0045e850((struct ObjNode *)elem, &pos.x);
                IncrementObjectCount((struct ObjectCount *)QueryClass);
                RemoveObjectFromMap(t);
            }
        } else {
            RemObjFromMap(QueryClass, (unsigned int)elem, t, &QueryCursor);
        }
        memcpy(&QueryCursor, &saved, sizeof(struct Cursor));
        QueryClass = saved_class;
        return;
    }
    if (tile->flags & 0x800) {
        order = GetGardenerWorkOrderAt(p->x, p->y);
        if (order != NULL) {
            FUN_0045e850((struct ObjNode *)order->element, &order->pos.x);
            FUN_0045d3d0((struct PathFootprint *)order->element->data, &order->pos.x);
            EraseGardenerOrder(order);
            return;
        }
        order = GetMechanicWorkOrderAt(p->x, p->y);
        if (order != NULL) {
            FUN_0045e850((struct ObjNode *)order->element, &order->pos.x);
            FUN_0045d3d0((struct PathFootprint *)order->element->data, &order->pos.x);
            EraseMechanicOrder(order);
            return;
        }
    } else if (tile->flags & 8) {
        RemovePathSquare(p);
        tile->flags &= 0xfff7;
        tile->field_10 &= 0xfe;
    }
}

// FUNCTION: LEGOLAND 0x00477bd0
void FindMapPathAStar(int x, int y, int a, int b) {
    register struct GameMainNode *best;
    struct GameMainNode *cur;
    register struct GameMainNode *nb;
    struct Element *id;
    struct MapElement *tile;
    int result;
    struct Point pos;
    struct Point *pp;
    int cost;

    unsigned int cur_x;
    int cur_y;
    int pos_x;
    int cur_x2;
    DAT_004bb598.x = x;
    DAT_004bb598.y = y;
    best = NULL;
    DAT_004bb5a0 = a;
    DAT_004bb5a4 = b;
    nb = FUN_004777f0(&DAT_004bb598, &result);
    nb->path_cost = 0;
    InsertOpenListSorted((struct EventNode *)nb);
    while ((cur = DAT_00668fc0) != NULL) {
        DAT_00668fc0 = cur->next;

        if ((DAT_004bb5a0 == cur->x && cur->y == DAT_004bb5a4) || cur->field_20 == 1) {
            best = cur;
        } else {
            pos.x = cur->x;
            pos.y = cur->y - 1;
            if (IsInMapBounds(pos.x, pos.y)) {
                nb = FUN_004777f0(&pos, &result);
                if (nb->step_cost != -1) {
                    if (nb->field_20 && (unsigned)nb->field_20 != 1) {
                        cur_y = cur->y;
                        cost = FUN_00477980(cur->field_24, FUN_004779a0(cur->x, cur_y, nb->x, nb->y)) + cur->path_cost + nb->step_cost;
                    } else {
                        cost = cur->path_cost + nb->step_cost;
                    }
                    if (result == 0) {
                        if (NULL != best && cost > best->path_cost) {
                            FUN_004776c0((struct QueryNode *)nb);
                        } else {
                            cur_x2 = cur->x;
                            nb->field_24 = FUN_004779a0(cur_x2, cur->y, nb->x, nb->y);
                            nb->parent = cur;
                            nb->path_cost = cost;
                            nb->field_1c = nb->heuristic + cost;
                            if (result == 2) {
                                RemoveQueryNode((struct QueryNode *)nb);
                            }
                            if (result == 1) {
                                RemoveFromOpenList((struct EventNode *)nb);
                            }
                            InsertOpenListSorted((struct EventNode *)nb);
                        }
                    }
                }
            }
            pos.x = cur->x + 1;
            pos.y = cur->y;
            if (IsInMapBounds(pos.x, pos.y)) {
                nb = FUN_004777f0(&pos, &result);
                cur_x = cur->x;
                nb->field_24 = FUN_004779a0(cur_x, cur->y, nb->x, nb->y);
                if (nb->step_cost != -1) {
                    if (!(0 != nb->field_20 && nb->field_20 != 1)) {
                        cost = cur->path_cost + nb->step_cost;
                    } else {
                        cost = FUN_00477980(cur->field_24, FUN_004779a0(cur->x, cur->y, nb->x, nb->y)) + cur->path_cost + nb->step_cost;
                    }
                    if (result == 0) {
                        if (NULL != best && cost > best->path_cost) {
                            FUN_004776c0((struct QueryNode *)nb);
                        } else {
                            nb->field_24 = FUN_004779a0(cur->x, cur->y, nb->x, nb->y);
                            nb->parent = cur;
                            nb->path_cost = cost;
                            nb->field_1c = nb->heuristic + cost;
                            if (result == 2) {
                                RemoveQueryNode((struct QueryNode *)nb);
                            }
                            if (result == 1) {
                                RemoveFromOpenList((struct EventNode *)nb);
                            }
                            InsertOpenListSorted((struct EventNode *)nb);
                        }
                    }
                }
            }
            pos_x = cur->x;
            pos.x = pos_x;
            pos.y = cur->y + 1;
            if (IsInMapBounds(pos.x, pos.y)) {
                nb = FUN_004777f0(&pos, &result);
                nb->field_24 = FUN_004779a0(cur->x, cur->y, nb->x, nb->y);
                if (nb->step_cost != -1) {
                    if (!(nb->field_20 != 0 && nb->field_20 != 1)) {
                        cost = cur->path_cost + nb->step_cost;
                    } else {
                        cost = FUN_00477980(cur->field_24, FUN_004779a0(cur->x, cur->y, nb->x, nb->y)) + cur->path_cost + nb->step_cost;
                    }
                    if (result == 0) {
                        if (NULL != best && cost > best->path_cost) {
                            FUN_004776c0((struct QueryNode *)nb);
                        } else {
                            nb->field_24 = FUN_004779a0(cur->x, cur->y, nb->x, nb->y);
                            nb->parent = cur;
                            nb->path_cost = cost;
                            nb->field_1c = nb->heuristic + cost;
                            if (result == 2) {
                                RemoveQueryNode((struct QueryNode *)nb);
                            }
                            if (result == 1) {
                                RemoveFromOpenList((struct EventNode *)nb);
                            }
                            InsertOpenListSorted((struct EventNode *)nb);
                        }
                    }
                }
            }
            pos.x = cur->x - 1;
            pos.y = cur->y;
            if (IsInMapBounds(pos.x, pos.y)) {
                nb = FUN_004777f0(&pos, &result);
                nb->field_24 = FUN_004779a0(cur->x, cur->y, nb->x, nb->y);
                if (nb->step_cost != -1) {
                    if (0 != nb->field_20 && nb->field_20 != 1) {
                        cost = FUN_00477980(cur->field_24, FUN_004779a0(cur->x, cur->y, nb->x, nb->y)) + cur->path_cost + nb->step_cost;
                    } else {
                        cost = cur->path_cost + nb->step_cost;
                    }
                    if (result == 0) {
                        if (best != NULL && cost > best->path_cost) {
                            FUN_004776c0((struct QueryNode *)nb);
                        } else {
                            nb->field_24 = FUN_004779a0(cur->x, cur->y, nb->x, nb->y);
                            nb->parent = cur;
                            nb->path_cost = cost;
                            nb->field_1c = nb->heuristic + cost;
                            if (2 == result) {
                                RemoveQueryNode((struct QueryNode *)nb);
                            }
                            if (result == 1) {
                                RemoveFromOpenList((struct EventNode *)nb);
                            }
                            InsertOpenListSorted((struct EventNode *)nb);
                        }
                    }
                }
            }
        }
        FUN_004776c0((struct QueryNode *)cur);
    }
    // STRING: LEGOLAND 0x004b8a70
    id = ElemID("PATH CONTROL");
    nb = best;
    while (nb) {
        pp = (struct Point *)&nb->x;
        if (0 <= pp->x && pp->x < lpConfig->width && pp->y >= 0 && pp->y < lpConfig->height) {
            tile = &GameMap[pp->y][pp->x];
        } else {
            tile = NULL;
        }
        if (!(tile->flags & 0x10)) {
            if ((tile->field_10 & 3) != 3) {
                FUN_004779d0(pp);
                AddBasicPath((struct EditObject *)id, (int *)pp);
            }
        }
        nb = nb->parent;
    }
    while (DAT_00668fc4 != NULL) {
        RemoveQueryNode((struct QueryNode *)DAT_00668fc4);
    }
}

// FUNCTION: LEGOLAND 0x00478110
int TokenizeString(char *str, const char *delims, char **out) {
    int count = 0;
    char *p;
    int n;

    while (*str) {
        str += strspn(str, delims);
        if (*str) {
            if (*str == '"') {
                *str = 0;
                str++;
                *out = str;
                count++;
                out++;
                p = strchr(str, '"');
                if (!p)
                    return count;
                *p = 0;
                str = p + 1;
            } else {
                *out = str;
                count++;
                out++;
                n = strcspn(str, delims);
                p = strchr(str, '"');
                if (p && p - str < n) {
                    str = p;
                } else {
                    str += n;
                    if (!*str)
                        return count;
                    *str = 0;
                    str++;
                }
            }
        }
    }
    return count;
}

// FUNCTION: LEGOLAND 0x004781b0
int FindStringNoCase(const char *param_1, const void *param_2, int param_3) {
    int i;

    for (i = 0; i < param_3; i++) {
        if (_strcmpi(param_1, ((const char **)param_2)[i]) == 0) {
            return i;
        }
    }
    return -1;
}
// FUNCTION: LEGOLAND 0x004781f0
int ParseScriptFile(const char *name, struct ScriptCommand *commands, int count, int flags) {
    char path[256];
    struct ResFile *file;
    int result;

    sprintf(path, "%s%s",
        // STRING: LEGOLAND 0x004bc084
        "Scripts\\", name);
    file = RES_OpenFile(path);
    if (file != NULL) {
        strncpy(DAT_00668fd0, name, 128);
        result = ParseScriptResFile(file, commands, count, flags);
        RES_CloseFile(file);
        return result;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00478280
int ParseScriptResFile(struct ResFile *file, struct ScriptCommand *commands, int count, int flags) {
    char line[1024];
    char *tokens[20];
    char *p;
    int r = 0;
    int errors = 0;
    ScriptCommandFn deflt = NULL;
    int ntok;
    int i;
    char found;
    struct ScriptCommand *cmd;

    DAT_00668fcc = 0;
    cmd = commands;
    // STRING: LEGOLAND 0x004bbdcc
    if (strcmp(cmd->name, "none") == 0)
        deflt = cmd->fn;
    while (ReadResFileLine(file, line, 1024)) {
        DAT_00668fcc++;
        DrawWatchSprite();
        p = strchr(line, '#');
        if (p)
            *p = 0;
        // STRING: LEGOLAND 0x004bc098
        ntok = TokenizeString(line, " ,;:(){}\xa0\n\t", tokens);
        if (ntok) {
            UppercaseStringAndGetLength(tokens[0]);
            found = 0;
            for (i = 0; i < count; i++, cmd++) {
                if (strcmp(tokens[0], cmd->name) == 0) {
                    found = 1;
                    r = commands[i].fn(tokens, ntok - 1, flags);
                    if (r == 0)
                        errors++;
                    break;
                }
            }
            cmd = commands;
            if (!found) {
                if (deflt)
                    r = deflt(tokens, ntok - 1, flags);
            }
        }
        if (r < 0)
            return r;
    }
    if (r < 0)
        return r;
    // STRING: LEGOLAND 0x004bc090
    if (strcmp(commands[count - 1].name, "check") == 0)
        r = commands[count - 1].fn(NULL, errors, flags);
    if (r < 0)
        return r;
    return errors;
}

// FUNCTION: LEGOLAND 0x004784c0
void FUN_004784c0(void) {
    NEWFLC_Repeat = 0;
    NEWFLC_BuffSize = 1;
    NEWFLC_CheckDuplicate = 1;
    DAT_00669054 = 0;
    DAT_004bb5ac = 1;
    CurrentObjectiveEventFlags = 0;
    CurrentScriptSection = 0;
    DAT_00669098 = 0;

    lpConfig->field_30 = 0;
    lpConfig->gardeners_enabled = 1;
    lpConfig->mechanics_enabled = 1;
    lpConfig->max_blokes = 0xc8;
    MapStats.capacity_max = lpConfig->max_blokes;
    MapStats.capacity_min = 0;

    FUN_004689a0();
    DAT_007fdca4 = FUN_004689f0(0, 0, 0);
    ScriptConditionActive = 1;

    FUN_004441f0();
    FUN_0044db20();
    StopAppraisalTimer();
    FUN_00468840();
    FUN_004688e0();
    FUN_0044dc70(0, 0);
    FUN_00468830();
    FUN_00482d70();
    FUN_00462e90();
    FUN_00476000();
    ClearButtonFlashStates();
    FUN_00490610(DAT_004d8bb0);
    FUN_00463560();
    ResetPathUpdateTimer();
    FUN_00459960();

    MapStats.brick_meter_max = 1000;
    MapStats.show_capacity = 0;
    MapStats.ride_wear = 0;
    MapStats.field_3a8 = 1;

    SetScriptStopped(0);
}

// FUNCTION: LEGOLAND 0x004785d0
void FUN_004785d0(char *param_1, unsigned int param_2) {
    strcpy(DAT_00669058, param_1);
    DAT_00669054 = param_2;
}

// FUNCTION: LEGOLAND 0x00478610
void SetScriptObjectiveKind(unsigned int param_1) {
    DAT_004bb5ac = param_1;
    switch (param_1) {
    case 1:
        CurrentObjectiveEventFlags = 1;
        break;
    case 2:
        CurrentObjectiveEventFlags = 2;
        break;
    case 3:
        CurrentObjectiveEventFlags = 4;
        break;
    default:
        CurrentObjectiveEventFlags = 0;
        break;
    }
}

// FUNCTION: LEGOLAND 0x00478650
void FUN_00478650(unsigned int param_1, unsigned int param_2) {
    // param_1 is an opaque command-block handle reinterpreted as a GameMainEntry
    struct GameMainEntry *entry = (struct GameMainEntry *)param_1;
    if (param_2 == 0) {
        return;
    }
    // STRING: LEGOLAND 0x004bb9ec
    if (_stricmp(entry->name, "PURGE") == 0) {
        CurrentObjectiveEventFlags |= 0x8;
    } else {
        CurrentObjectiveEventFlags &= 0xf7;
    }
}

// FUNCTION: LEGOLAND 0x00478690
int FUN_00478690(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    return (int)param_2 >= (int)param_3;
}

// FUNCTION: LEGOLAND 0x004786a0
int FUN_004786a0(unsigned int param_1, unsigned int param_2, unsigned int param_3) {
    return (DAT_00669054 & param_3) != 0;
}

// FUNCTION: LEGOLAND 0x004786c0
unsigned int FUN_004786c0(unsigned int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4) {
    if (FUN_004786a0(param_1, param_2, param_3) == 0) {
        return 0;
    }
    return FUN_00478690(param_1, param_2, param_4) != 0;
}

// FUNCTION: LEGOLAND 0x00478700
void ParseRect(int *param_1, char **param_2, int param_3) {
    int temp;

    param_1[0] = atoi(param_2[param_3 + 0]);
    param_1[1] = atoi(param_2[param_3 + 1]);
    param_1[2] = atoi(param_2[param_3 + 2]);
    param_1[3] = atoi(param_2[param_3 + 3]);

    if (param_1[0] > param_1[2]) {
        temp = param_1[0];
        param_1[0] = param_1[2];
        param_1[2] = temp;
    }
    if (param_1[1] > param_1[3]) {
        temp = param_1[1];
        param_1[1] = param_1[3];
        param_1[3] = temp;
    }
}

// FUNCTION: LEGOLAND 0x00478770
void ParseIntPair(int *param_1, char **param_2, int param_3) {
    param_1[0] = atoi(param_2[param_3 + 0]);
    param_1[1] = atoi(param_2[param_3 + 1]);
}

// FUNCTION: LEGOLAND 0x004787a0
unsigned int FUN_004787a0(unsigned int param_1, unsigned int param_2) {
    // param_1 is an opaque command-block handle reinterpreted as a GameMainArg
    return ((struct GameMainArg *)param_1)->field_4;
}
