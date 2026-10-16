/* [library:filesystem] The first-run setup off Windows (port_setup_win.c is Windows'): when the game's data isn't
 * there, find the LEGOLAND CD (an image file next to the program or in the current folder, or a mounted CD) or let
 * the player choose one with SDL's file dialogs, then build the data folder next to the program with
 * PortSetupInstall, a progress bar in an SDL window meanwhile. Messages are SDL message boxes. */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#ifdef LEGOLAND_WEB_BROWSER
#include <emscripten.h>
#endif

#include "port_setup.h"
#include "port_setup_win.h"
#include "port_trace.h"

#define PATH_SIZE 1024

/* ---- finding the CD ---- */

struct Search {
    char found[PATH_SIZE];
};

static SDL_EnumerationResult SDLCALL Look(void *userdata, const char *dirname, const char *fname) {
    struct Search *s = (struct Search *)userdata;
    char path[PATH_SIZE];
    int kind;

    SDL_snprintf(path, sizeof(path), "%s%s", dirname, fname);
    kind = PortSetupKind(path);
    if (kind == PORT_SETUP_IMAGE || kind == PORT_SETUP_CD) {
        SDL_strlcpy(s->found, path, sizeof(s->found));
        return SDL_ENUM_SUCCESS;
    }
    return SDL_ENUM_CONTINUE;
}

/* an image or a CD in folder, or the folder itself being the CD */
static int LookIn(const char *folder, char *out) {
    struct Search s;

    if (folder == NULL || folder[0] == '\0') {
        return 0;
    }
    if (PortSetupKind(folder) == PORT_SETUP_CD) {
        SDL_strlcpy(out, folder, PATH_SIZE);
        return 1;
    }
    s.found[0] = '\0';
    SDL_EnumerateDirectory(folder, Look, &s);
    if (s.found[0] != '\0') {
        SDL_strlcpy(out, s.found, PATH_SIZE);
        return 1;
    }
    return 0;
}

/* the mount points' folders (each mounted disc is one of their entries) */
static SDL_EnumerationResult SDLCALL LookMounted(void *userdata, const char *dirname, const char *fname) {
    char path[PATH_SIZE];

    SDL_snprintf(path, sizeof(path), "%s%s/", dirname, fname);
    return LookIn(path, (char *)userdata) ? SDL_ENUM_SUCCESS : SDL_ENUM_CONTINUE;
}

static int FindSource(char *out) {
    const char *user = SDL_getenv("USER");
    char path[PATH_SIZE];
    char *cwd = SDL_GetCurrentDirectory();
    int found;

    found = LookIn(PortSetupExeFolder(), out) || LookIn(cwd, out);
    SDL_free(cwd);
    if (!found && user != NULL) {
        out[0] = '\0';
        SDL_snprintf(path, sizeof(path), "/media/%s/", user);
        SDL_EnumerateDirectory(path, LookMounted, out);
        if (out[0] == '\0') {
            SDL_snprintf(path, sizeof(path), "/run/media/%s/", user);
            SDL_EnumerateDirectory(path, LookMounted, out);
        }
        found = out[0] != '\0';
    }
    if (!found) {
        found = LookIn("/media/cdrom/", out) || LookIn("/cdrom/", out) || LookIn("/mnt/cdrom/", out);
    }
    return found;
}

/* ---- choosing it ---- */

struct Choice {
    int done;
    char path[PATH_SIZE];
};

static void SDLCALL Chosen(void *userdata, const char *const *files, int filter) {
    struct Choice *c = (struct Choice *)userdata;

    (void)filter;
    if (files != NULL && files[0] != NULL) {
        SDL_strlcpy(c->path, files[0], sizeof(c->path));
    }
    c->done = 1;
}

static int Choose(int folder, char *out) {
    static const SDL_DialogFileFilter filters[] = {{"CD images (*.iso, *.bin)", "iso;bin"}, {"All files", "*"}};
    struct Choice c;

    c.done = 0;
    c.path[0] = '\0';
    if (folder) {
        SDL_ShowOpenFolderDialog(Chosen, &c, NULL, NULL, false);
    } else {
        SDL_ShowOpenFileDialog(Chosen, &c, NULL, filters, 2, NULL, false);
    }
    while (!c.done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
        }
        SDL_Delay(10);
    }
    if (c.path[0] == '\0') {
        return 0;
    }
    SDL_strlcpy(out, c.path, PATH_SIZE);
    return 1;
}

static int Ask(const char *text, const char *const *buttons, int count) {
    SDL_MessageBoxButtonData data[4];
    SDL_MessageBoxData box;
    int pressed = -1;
    int i;

    SDL_zero(box);
    for (i = 0; i < count; i++) {
        data[i].flags = i == 0 ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT
            : i == count - 1   ? SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT
                               : 0;
        data[i].buttonID = i;
        data[i].text = buttons[i];
    }
    box.flags = SDL_MESSAGEBOX_INFORMATION;
    box.title = "LEGOLAND";
    box.message = text;
    box.numbuttons = count;
    box.buttons = data;
    if (!SDL_ShowMessageBox(&box, &pressed)) {
        fprintf(stderr, "LEGOLAND: %s\n", text);
        return -1;
    }
    return pressed;
}

/* ---- installing, with a progress bar ---- */

struct Progress {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int cancelled;
    Uint64 last;
};

static void Report(void *ctx, unsigned long long done, unsigned long long total, const char *file) {
    struct Progress *p = (struct Progress *)ctx;
    SDL_Event e;
    SDL_FRect bar;
    Uint64 now = SDL_GetTicks();

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            p->cancelled = 1;
        }
    }
    if (p->renderer == NULL || (now - p->last < 50 && done < total)) {
        return;
    }
    p->last = now;
    SDL_SetRenderDrawColor(p->renderer, 240, 240, 240, 255);
    SDL_RenderClear(p->renderer);
    SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 255);
    SDL_RenderDebugText(p->renderer, 20, 20, "Installing LEGOLAND from the CD...");
    SDL_RenderDebugText(p->renderer, 20, 40, file != NULL ? file : "");
    bar.x = 20;
    bar.y = 70;
    bar.w = 440;
    bar.h = 20;
    SDL_RenderRect(p->renderer, &bar);
    SDL_SetRenderDrawColor(p->renderer, 0, 120, 215, 255);
    bar.w = total > 0 ? (float)(440.0 * (double)done / (double)total) : 0;
    SDL_RenderFillRect(p->renderer, &bar);
    SDL_RenderPresent(p->renderer);
#ifdef LEGOLAND_WEB_BROWSER
    /* the install runs on the page's thread: give the page its turn, so that it shows the bar (ASYNCIFY) */
    emscripten_sleep(0);
#endif
}

static int Cancelled(void *ctx) {
    return ((struct Progress *)ctx)->cancelled;
}

static int Install(const char *source) {
    struct Progress p;
    PortSetupProgress progress;
    char error[512];
    char text[PATH_SIZE + 600];
    int ok;

    SDL_zero(p);
    if (SDL_CreateWindowAndRenderer("LEGOLAND setup", 480, 110, 0, &p.window, &p.renderer)) {
        Report(&p, 0, 1, "");
    }
    progress.report = Report;
    progress.cancelled = Cancelled;
    progress.ctx = &p;
    PortTrace("setup: installing from %s into %s", source, PortSetupExeFolder());
    ok = PortSetupInstall(source, PortSetupExeFolder(), &progress, error, sizeof(error));
    if (p.renderer != NULL) {
        SDL_DestroyRenderer(p.renderer);
    }
    if (p.window != NULL) {
        SDL_DestroyWindow(p.window);
    }
    if (!ok) {
        static const char *const buttons[] = {"OK"};
        SDL_snprintf(text, sizeof(text), "LEGOLAND couldn't be installed from %s:\n%s", source, error);
        PortTrace("setup: failed: %s", error);
        Ask(text, buttons, 1);
    }
    return ok;
}

int PortSetupRun(HINSTANCE instance) {
    char source[PATH_SIZE];
    char text[PATH_SIZE + 300];

    (void)instance;
    SDL_InitSubSystem(SDL_INIT_VIDEO);
#ifdef __EMSCRIPTEN__
    /* the page (port/web/shell.html) copies the CD's image the player chose into the game's folder before starting
     * the game: install from it, then remove it (it's the size of the CD) */
    if (FindSource(source) && PortSetupKind(source) == PORT_SETUP_IMAGE) {
        int ok = Install(source);
        if (ok) {
            remove(source); /* only once it's installed: after a failed install the player needn't choose it again */
        }
        return ok && PortSetupFindData();
    }
    fprintf(stderr, "LEGOLAND: the game's files aren't installed: choose the CD's image on the page\n");
    return 0;
#endif
    if (FindSource(source)) {
        static const char *const buttons[] = {"Install", "Choose another...", "Quit"};
        int answer;

        SDL_snprintf(text, sizeof(text),
            "LEGOLAND's game files will be installed from the CD here:\n%s\ninto the program's folder:\n%s",
            source, PortSetupExeFolder());
        answer = Ask(text, buttons, 3);
        if (answer == 0 || answer == -1) {
            return Install(source) && PortSetupFindData();
        }
        if (answer != 1) {
            return 0;
        }
    }
    for (;;) {
#ifdef __ANDROID__
        /* Android's file dialog chooses files, not folders: the CD's image; an installed copy can be copied into the
         * app's folder instead */
        static const char *const buttons[] = {"Choose the CD image...", "Quit"};
        int answer;

        SDL_snprintf(text, sizeof(text),
            "LEGOLAND needs the game files from its CD. Choose the CD's image (an .iso or .bin file); they'll be put "
            "in the app's folder:\n%s\n(Or copy an installed LEGOLAND folder's contents there yourself.)",
            PortSetupExeFolder());
        answer = Ask(text, buttons, 2);
        if (answer == 1) {
            answer = 2;
        }
#else
        static const char *const buttons[] = {"Choose ISO...", "Choose folder...", "Quit"};
        int answer;

        SDL_snprintf(text, sizeof(text),
            "LEGOLAND needs the game files from its CD. Choose the CD (or its image, an .iso or .bin file), or a "
            "folder where it's copied or installed. They'll be put in the program's folder:\n%s",
            PortSetupExeFolder());
        answer = Ask(text, buttons, 3);
#endif
        if (answer != 0 && answer != 1) {
            return 0;
        }
        if (Choose(answer == 1, source)) {
            if (PortSetupKind(source) == PORT_SETUP_NONE) {
                static const char *const ok[] = {"OK"};
                SDL_snprintf(text, sizeof(text), "That isn't the LEGOLAND CD:\n%s", source);
                Ask(text, ok, 1);
                continue;
            }
            return Install(source) && PortSetupFindData();
        }
    }
}
