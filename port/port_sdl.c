#include "port_sdl.h"

#include <SDL3/SDL.h>
#include <stdio.h>

int PortSdlSelfTest(const char *path) {
    FILE *f = fopen(path, "w");
    int linked = SDL_GetVersion();
    int bad = 0;
    int i;

    if (f == NULL) {
        return -1;
    }
    fprintf(f, "SDL %d.%d.%d linked (%s), built against %d.%d.%d\n", SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked), SDL_GetRevision(), SDL_MAJOR_VERSION,
        SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    if (linked != SDL_VERSION) {
        fprintf(f, "SDL: the library isn't the version of its headers\n");
        bad++;
    }
    fprintf(f, "SDL video drivers:");
    for (i = 0; i < SDL_GetNumVideoDrivers(); i++) {
        fprintf(f, " %s", SDL_GetVideoDriver(i));
    }
    fprintf(f, "\n");
    fclose(f);
    return bad;
}
