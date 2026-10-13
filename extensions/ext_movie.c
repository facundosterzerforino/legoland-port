/* Pause movies: see ext_movie.h. */
#include <SDL3/SDL.h>

#include "ext_movie.h"
#include "miniwin/miniwin.h"

int ExtPauseMovies = 0;

static int waiting_now;
static Uint64 wait_started;
static Uint64 waited;

void ExtPauseMoviesApply(void) {
    MiniwinSoundHoldWhenInactive(ExtPauseMovies);
}

void ExtPauseMoviesWaiting(int waiting) {
    if (waiting && !waiting_now) {
        waiting_now = 1;
        wait_started = SDL_GetTicks();
    } else if (!waiting && waiting_now) {
        waiting_now = 0;
        waited += SDL_GetTicks() - wait_started;
    }
}

int ExtPauseMoviesTake(void) {
    int ms = ExtPauseMovies ? (int)waited : 0;

    waited = 0;
    return ms;
}
