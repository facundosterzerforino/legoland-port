/* The picture on the screen (miniwin/src/window.c, MiniwinPresent on SDL3's renderer): a test picture of four
 * colours, red, green, blue and white quarters, shown for 300 frames, each frame's time measured, then shown for a few
 * seconds more for a look at the screen (and a change of the window's size), then the input: a click, a key and a
 * typed character (MiniwinTypeChar, as the web page's text box sends them) must reach the game (in a browser: the page
 * does them, tools/web/presenttest.py). In the browser build (tools/test_miniwin/build_web.sh, run by
 * tools/web/presenttest.py) this is the web page's drawing as the game does it: the game's thread is a worker's and its
 * WebGL calls are made on the page's thread (port/web/README.md). Off Windows also:
 *
 *     SDL_VIDEO_DRIVER=offscreen build-linux/tests/test_present
 */
#include <SDL3/SDL.h>
#include <stdio.h>

#include "miniwin/miniwin.h"

#define WIDTH 640
#define HEIGHT 480
#define FRAMES 300

static unsigned short pixels[HEIGHT][WIDTH];

static LRESULT CALLBACK Proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void)hwnd;
    (void)msg;
    (void)wparam;
    (void)lparam;
    return 0;
}

static void Show(void) {
    RECT dest;
    int width = WIDTH;
    int height = HEIGHT;

    MiniwinPumpEvents(Proc);
    MiniwinOutputSize(&width, &height);
    dest.left = 0;
    dest.top = 0;
    dest.right = width;
    dest.bottom = height;
    MiniwinPresent(pixels, WIDTH * 2, WIDTH, HEIGHT, &dest);
}

int main(int argc, char **argv) {
    Uint64 total = 0;
    Uint64 slowest = 0;
    Uint64 start;
    int width = 0;
    int height = 0;
    int x;
    int y;
    int i;

    (void)argc;
    (void)argv;
    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            /* RGB565: red, green above; blue, white below */
            pixels[y][x] = y < HEIGHT / 2 ? (x < WIDTH / 2 ? 0xf800 : 0x07e0) : (x < WIDTH / 2 ? 0x001f : 0xffff);
        }
    }
    if (MiniwinCreateWindow("miniwin present test", WIDTH, HEIGHT, MINIWIN_WINDOW) == NULL) {
        printf("present: no window: %s\nFAILED\n", MiniwinError());
        return 1;
    }
    printf("present: renderer %s\n", MiniwinRendererName());
    for (i = 0; i < FRAMES; i++) {
        Uint64 took;

        start = SDL_GetTicksNS();
        Show();
        took = SDL_GetTicksNS() - start;
        total += took;
        if (took > slowest) {
            slowest = took;
        }
    }
    MiniwinOutputSize(&width, &height);
    printf("present: %dx%d pixels, %d frames: %.2f ms a frame, the slowest %.2f ms\n", width, height, FRAMES,
        total / 1e6 / FRAMES, slowest / 1e6);
#ifdef __EMSCRIPTEN__
    printf("present: showing\n");
    fflush(stdout);
    start = SDL_GetTicksNS();
    while (SDL_GetTicksNS() - start < 6000000000ull) {
        Show();
        SDL_Delay(30);
    }
    MiniwinOutputSize(&width, &height);
    printf("present: now %dx%d pixels\n", width, height);
    printf("present: waiting for input\n");
    fflush(stdout);
    {
        int clicked = 0;
        int keyed = 0;
        int typed = 0;

        start = SDL_GetTicksNS();
        while ((!clicked || !keyed || !typed) && SDL_GetTicksNS() - start < 20000000000ull) {
            int shift;
            int code;
            int mx = 0;
            int my = 0;

            Show();
            if (!clicked && (MiniwinMouseState(&mx, &my) & 1) != 0) {
                printf("input: click at %d,%d\n", mx, my);
                clicked = 1;
            }
            if (!keyed && MiniwinKeyLatched(SDL_SCANCODE_A)) {
                printf("input: key A\n");
                keyed = 1;
            }
            code = MiniwinTypedKey(&shift);
            if (!typed && code != 0) {
                printf("input: typed key %d%s\n", code, shift ? " with Shift" : "");
                typed = 1;
            }
            fflush(stdout);
            SDL_Delay(15);
        }
        if (!clicked || !keyed || !typed) {
            printf("input: missing:%s%s%s\n", clicked ? "" : " the click", keyed ? "" : " the key",
                typed ? "" : " the typed character");
        }
    }
#endif
    printf("present: done\n");
    fflush(stdout);
    return 0;
}
