#include "dialog.h"
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clipping.h"
#include "controller.h"
#include "draw.h"
#include "gfx.h"
#include "globals.h"
#include "image_sprite.h"
#include "input.h"
#include "legoland.h"
#include "llidb.h"
#include "print_sprite.h"
#include "render.h"
#include "text.h"
#include "wndenv.h"

// FUNCTION: LEGOLAND 0x0043e930
int FUN_0043e930(RECT *rc, int min, int max, int value, int step) {
    int pos;

    pos = (rc->bottom - rc->top) * value / (max - min);
    RenderThickBox(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, 2, 0);
    RenderBlock(rc->left, rc->top + pos - 1, rc->right - rc->left, 3, GetNearestColour(0xff, 0, 0));
    if ((DAT_00813ac4 & 4) && DAT_00813a44.x >= rc->left && DAT_00813a44.x <= rc->right &&
        DAT_00813a44.y >= rc->top && DAT_00813a44.y <= rc->bottom) {
        DAT_0062fea4 = 1;
    } else if (!DAT_0062fea4) {
        return value;
    }
    if (DAT_00813ac4 & 4) {
        pos = DAT_00813a44.y;
        if (pos < rc->top) {
            pos = rc->top;
        } else if (pos > rc->bottom) {
            pos = rc->bottom;
        }
        return (pos - rc->top) * (max - min) / (rc->bottom - rc->top);
    }
    DAT_0062fea4 = 0;
    return value;
}

// FUNCTION: LEGOLAND 0x0043ea30
int FUN_0043ea30(char **names, char *title, struct Sprite *bg, RECT *box, void (*callback)(int), struct Sprite **icons, int w, int h, int flag) {
    RECT clip;
    RECT bar;
    RECT saved;
    RECT *items;
    int sel;
    int retry;
    int n;
    int i;
    int j;
    int y;
    int tw;
    int itemW;
    int viewH;
    int over;
    int my;
    int l;
    int t;
    char **np;
    RECT *ip;

    sel = -1;
    clip.left = box->left + 8;
    clip.top = box->top + 32;
    clip.right = box->right + box->left - 9;
    clip.bottom = box->top + box->bottom - 9;
    retry = 0;
    DAT_0062fea4 = 0;
    if (flag == 0) {
        DAT_0062fea0 = 0;
    }
    n = 0;
    while (names[n]) {
        n++;
    }
    items = malloc(n * 16);
    viewH = clip.bottom - clip.top + 1;
    for (;;) {
        itemW = clip.right - clip.left - 1;
        y = 0;
        for (i = 0; i < n; i++) {
            items[i].top = y;
            tw = MeasureTextHeight(names[i], 2, itemW);
            if (tw < 8) {
                tw = 8;
            } else if (tw < 0) {
                tw = 0;
            }
            y += tw;
            items[i].bottom = y;
            y += 4;
        }
        if (!retry && y > viewH) {
            retry = 1;
            clip.right -= 16;
            continue;
        }
        break;
    }
    over = y - viewH;
    for (i = 0; i < n; i++) {
        items[i].left = 0;
        items[i].right = itemW;
    }
    bar.left = box->right + box->left - 20;
    bar.top = clip.top;
    bar.right = box->right + box->left - 5;
    bar.bottom = clip.bottom + 0;
    for (;;) {
        if (!ProcessSystemEvents()) {
            break;
        }
        ReadGameButtons();
        if ((DAT_00813ad4 & 1) || (DAT_00813acc & 1)) {
            break;
        }
        if (DAT_00813a44.x >= clip.left && DAT_00813a44.x <= clip.right && DAT_00813a44.y >= clip.top && DAT_00813a44.y <= clip.bottom) {
            my = DAT_00813a44.y - clip.top + DAT_0062fea0;
            for (i = 0; i < n; i++) {
                if (my >= items[i].top && my <= items[i].bottom) {
                    sel = i;
                    if (!DAT_0062fea4 && (DAT_00813ac4 & 2)) {
                        return i;
                    }
                    break;
                }
            }
            if (i == n) {
                sel = -1;
            }
        } else {
            sel = -1;
        }
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(bg, 0, 0, 0, 0);
        if (callback) {
            callback(sel);
        }
        RenderBlock(box->left, box->top, box->right, box->bottom, GetNearestColour(0xef, 0xef, 0xef));
        RenderThickBox(box->left, box->top, box->right, box->bottom, 2, 0);
        RenderBlock(box->left + 2, box->top + 2, box->right - 4, 0x18, GetNearestColour(0, 0x3f, 0x7f));
        PrintLimitedText(box->left + 2, box->top + 2, box->right - 4, title, 0, 0xefefef, 0);
        if (retry) {
            DAT_0062fea0 = FUN_0043e930(&bar, 0, over, DAT_0062fea0, 0x10);
        }
        GetClipping(&saved);
        SetClipping(&clip);
        for (j = 0; j < n; j++) {
            if (items[j].bottom >= DAT_0062fea0) {
                break;
            }
        }
        if (j < n) {
            np = names + j;
            for (i = j; i < n; i++) {
                ip = &items[i];
                if (ip->top >= DAT_0062fea0 + viewH) {
                    break;
                }
                t = ip->top;
                l = ip->left;
                if (sel == i) {
                    RenderBlock(l + clip.left, t - DAT_0062fea0 + clip.top, ip->right - l + 1, ip->bottom - t + 1, GetNearestColour(0x7f, 0x7f, 0xef));
                } else {
                    RenderBox(l + clip.left, t - DAT_0062fea0 + clip.top, ip->right - l + 1, ip->bottom - t + 1, GetNearestColour(0xcf, 0xcf, 0xcf));
                }
                FUN_00455220(ip->left + clip.left, ip->top - DAT_0062fea0 + clip.top, *np, 2, itemW);
                np++;
            }
        }
        SetClipping(&saved);
        RenderingComplete();
        PopRenderingStatus();
    }
    return -1;
}

// FUNCTION: LEGOLAND 0x0043eee0
struct Element *FUN_0043eee0(char *title, struct Sprite *bg, RECT *box, unsigned int mask, int flag) {
    struct Sprite *happy;
    struct Sprite *poor;
    struct Element *e;
    struct Element *t;
    struct Element *u;
    struct Element **list;
    char **names;
    struct Sprite **icons;
    struct Element *result;
    int count;
    int n;
    int c;
    int i;
    int j;
    int k;
    int swapped;
    int r;

    count = LLIDB_GetCount();
    c = 0;
    // STRING: LEGOLAND 0x004b7a84
    happy = LoadSprite("happy.lls", 0);
    poor = LoadSprite("poor.lls", 0);
    for (i = 0; i < count; i++) {
        LLIDB_GetElement(i, &e);
        if (e->flags & mask) {
            c++;
        }
    }
    names = malloc(c * 4 + 4);
    list = malloc(c * 4);
    icons = malloc(c * 4);
    n = 0;
    for (k = 0; k < count; k++) {
        LLIDB_GetElement(k, &e);
        if (e->flags & mask) {
            list[n] = e;
            n++;
        }
    }
    names[n] = 0;
    for (i = 0; i < n - 1; i++) {
        swapped = 0;
        for (j = n - 2; j >= i; j--) {
            if (_strcmpi(list[j + 1]->name, list[j]->name) < 0) {
                t = list[j];
                u = list[j + 1];
                list[j + 1] = t;
                list[j] = u;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
    for (k = 0; k < n; k++) {
        names[k] = list[k]->name;
        if (list[k]->flags & 1) {
            icons[k] = happy;
        } else {
            icons[k] = poor;
        }
    }
    r = FUN_0043ea30(names, title, bg, box, 0, icons, 0x2e, 0x28, flag);
    if (r != -1) {
        result = list[r];
    } else {
        result = 0;
    }
    free(names);
    free(list);
    free(icons);
    KillSprite(happy);
    KillSprite(poor);
    return result;
}

// FUNCTION: LEGOLAND 0x0043f0b0
char *FUN_0043f0b0(char *title, struct Sprite *bg, RECT *box, char *path) {
    struct Sprite *drive;
    struct Sprite *folder;
    struct Sprite *file;
    char cwd[260];
    char drv[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char spec[260];
    struct _finddata_t fd;
    struct FileNode *head;
    struct FileNode *tail;
    struct FileNode *node;
    struct FileNode *a;
    struct FileNode *b;
    struct FileNode **nodes;
    char **names;
    struct Sprite **icons;
    char *result;
    long h;
    int count;
    int chdirFlag;
    int i;
    int j;
    int swapped;
    int r;

    tail = 0;
    // STRING: LEGOLAND 0x004b7ab0
    drive = LoadSprite("Drive.bmp", 0);
    // STRING: LEGOLAND 0x004b7aa4
    folder = LoadSprite("Folder.bmp", 0);
    // STRING: LEGOLAND 0x004b7a98
    file = LoadSprite("File.bmp", 0);
    _getcwd(cwd, 260);
    _splitpath(path, drv, dir, fname, ext);
    _chdir(dir);
    // STRING: LEGOLAND 0x004b7a90
    sprintf(spec, "%s%s", fname, ext);
    count = 0;
    chdirFlag = 0;
    h = _findfirst(spec, &fd);
    while (h != -1) {
        do {
            if (tail) {
                tail->next = malloc(12);
                tail = tail->next;
            } else {
                head = malloc(12);
                tail = head;
            }
            count++;
            tail->name = malloc(strlen(fd.name) + 1);
            strcpy(tail->name, fd.name);
            tail->attrib = fd.attrib;
        } while (_findnext(h, &fd) != -1);
        _findclose(h);
        h = count;
        tail->next = 0;
        names = malloc(h * 4 + 4);
        icons = malloc(h * 4);
        nodes = malloc(h * 4);
        names[h] = 0;
        node = head;
        for (i = 0; i < h; i++) {
            nodes[i] = node;
            node = node->next;
        }
        for (i = 0; i < h - 1; i++) {
            swapped = 0;
            for (j = h - 2; j >= i; j--) {
                a = nodes[j + 1];
                b = nodes[j];
                if (((b->attrib ^ a->attrib) >> 4 & 1) != 0) {
                    if (!(a->attrib & 0x10)) {
                        continue;
                    }
                } else if (_strcmpi(a->name, b->name) >= 0) {
                    continue;
                }
                nodes[j + 1] = b;
                nodes[j] = a;
                swapped = 1;
            }
            if (!swapped) {
                break;
            }
        }
        for (i = 0; i < h; i++) {
            names[i] = nodes[i]->name;
            icons[i] = (nodes[i]->attrib & 0x10) ? folder : file;
        }
        r = FUN_0043ea30(names, title, bg, box, 0, icons, 0x1c, 0x18, 0);
        if (r != -1) {
            if (nodes[r]->attrib & 0x10) {
                chdirFlag = 1;
            }
            strcpy(DAT_0081c8e0, names[r]);
            result = DAT_0081c8e0;
        } else {
            result = 0;
        }
        free(names);
        free(icons);
        for (i = 0; i < h; i++) {
            free(nodes[i]);
        }
        free(nodes);
        if (chdirFlag == 0) {
            KillSprite(drive);
            KillSprite(folder);
            KillSprite(file);
            _chdir(cwd);
            return result;
        }
        _chdir(DAT_0081c8e0);
        chdirFlag = 0;
        result = 0;
        count = 0;
        tail = 0;
        h = _findfirst(spec, &fd);
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0043f460
int FUN_0043f460(RECT *rc, int unused, char *buf, int maxlen, int *pos) {
    char c;

    c = GetInputChar();
    if (c != 0) {
        switch (c) {
        case (char)0xfd:
            return 1;
        case (char)0xfe:
            return 0;
        case (char)0xff:
            if (*pos > 0) {
                (*pos)--;
            }
            buf[*pos] = 0;
            break;
        default:
            if (*pos < maxlen - 1) {
                buf[*pos] = c;
                (*pos)++;
                buf[*pos] = 0;
            }
            break;
        }
    }
    PrintLimitedText(rc->left + 4, rc->top + 4, rc->right - rc->left - 8, buf, 2, 0, 0);
    return 0;
}

// FUNCTION: LEGOLAND 0x0043f4f0
int FUN_0043f4f0(struct Sprite *bg, RECT *box, char *title, char *buf, int maxlen) {
    int pos;
    RECT rc;
    int done;

    pos = strlen(buf);
    rc.left = box->left + 8;
    rc.top = box->top + 0x20;
    rc.right = box->right + box->left - 9;
    rc.bottom = box->top + box->bottom - 9;
    done = 0;
    while (!done && ProcessSystemEvents()) {
        ReadGameButtons();
        PushRenderingStatusAndLockVideoSurface();
        PrintSprite(bg, 0, 0, 0, 0);
        RenderBlock(box->left, box->top, box->right, box->bottom, GetNearestColour(0xef, 0xef, 0xef));
        RenderThickBox(box->left, box->top, box->right, box->bottom, 2, 0);
        RenderBlock(box->left + 2, box->top + 2, box->right - 4, 0x18, GetNearestColour(0, 0x3f, 0x7f));
        PrintLimitedText(box->left + 2, box->top + 2, box->right - 4, title, 0, 0xefefef, 0);
        RenderThickBox(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 2, 0);
        done = FUN_0043f460(&rc, 0, buf, maxlen, &pos);
        RenderingComplete();
        PopRenderingStatus();
    }
    return pos;
}
