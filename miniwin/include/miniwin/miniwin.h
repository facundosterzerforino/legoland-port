#ifndef MINIWIN_MINIWIN_H
#define MINIWIN_MINIWIN_H

/* miniwin: the Windows APIs the game calls, reimplemented so that it can run without DirectX (PORTING.md).
 * For now it is built on Windows only, next to the real SDK headers: the game creates its DirectDraw object and
 * window here instead of with DirectDrawCreate and CreateWindowEx, and every other call reaches this code through
 * the same COM interfaces as before. */

#include <windows.h>
#include <ddraw.h>
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0500 /* the game's (input.c) */
#endif
#include <dinput.h>
#include <dsound.h>

/* [library:video] DirectDraw on memory surfaces (miniwin/src/ddraw.c), called like DirectDrawCreate. It has what
 * the game uses: 16-bit RGB565 surfaces; Blt copies, stretches and colour fills, with source colour keys and clip
 * lists; Lock; GetDC (GDI drawing on the surface, Windows only). Nothing reaches the screen by itself: the frame is
 * shown with MiniwinPresent. */
HRESULT WINAPI MiniwinDirectDrawCreate(GUID *guid, LPDIRECTDRAW *out, IUnknown *outer);

/* [library:window] The game's window, made by SDL3 (miniwin/src/window.c). */
enum {
    MINIWIN_WINDOW, /* a window with a client area of width x height */
    MINIWIN_FULLSCREEN_DESKTOP, /* full screen at the desktop's resolution */
    MINIWIN_FULLSCREEN_MODE /* full screen in the display mode closest to width x height */
};

/* Creates the window and its renderer; returns its HWND (which DirectSound still needs), or NULL (MiniwinError
 * says why). The system cursor is hidden over it: the game draws its own. */
HWND MiniwinCreateWindow(const char *title, int width, int height, int fullscreen);

/* The SDL_Window behind the game window's HWND, or NULL for any other window. */
void *MiniwinWindowOf(HWND hwnd);
/* SDL's last error message. */
const char *MiniwinError(void);

/* The renderer SDL chose (e.g. "direct3d11"). */
const char *MiniwinRendererName(void);

/* Hands SDL's window events to the game's window procedure as the window messages it handles: focus lost and
 * gained (WM_KILLFOCUS, WM_SETFOCUS), the mouse wheel (WM_MOUSEWHEEL), Backspace (WM_CHAR 8) and the close button
 * (WM_CLOSE). Call it from the game's message loop. */
void MiniwinPumpEvents(WNDPROC proc);

/* The size of the window's picture in pixels. Returns 0 if there is no window. */
int MiniwinOutputSize(int *width, int *height);

/* Shows a frame: width x height RGB565 pixels, scaled (nearest neighbour) into dest, black around it. */
void MiniwinPresent(const void *pixels, int pitch, int width, int height, const RECT *dest);

/* 1 while the window has the keyboard focus. */
int MiniwinHasFocus(void);

/* 1 while the window exists but another one has the focus (the game waits then, and its sounds are muted). */
int MiniwinWindowInactive(void);

/* The mouse: its position in the window's picture in pixels (x, y may be NULL), and the buttons that are down
 * (bit 0 the primary button, bit 1 the secondary, bit 2 the middle one). */
unsigned int MiniwinMouseState(int *x, int *y);

/* The buttons for the game's read of the mouse, once a frame. As MiniwinMouseState, but on a touch screen a finger's
 * press reaches the game one read after the pointer moved to it, and lasts at least two reads (a tap shorter than a
 * frame is still a click). */
unsigned int MiniwinMouseButtons(void);

/* Touch gestures. One finger that starts where area(x, y) says (window pixels; a list) and moves is a drag: its
 * movement is taken by MiniwinTouchTakeDrag and the game gets no click (a tap there is still a click, a still press a
 * press). Two fingers moving are a pan; held still, the secondary button; lifted at once, a secondary click. */
void MiniwinSetTouchDragArea(int (*area)(int x, int y));

/* The one-finger drag's and the two-finger pan's movement since the last call, in window pixels. */
void MiniwinTouchTakeDrag(float *dx, float *dy, float *pan_x, float *pan_y);

/* The fingers on the screen now. */
int MiniwinTouchFingers(void);

/* The presses so far (mouse buttons, fingers, keys): a changed count means something was pressed since. */
unsigned int MiniwinPressCount(void);

/* 1 while the last pointer input was a finger or a pen (Android, and a phone's browser, start so): the game's own
 * pointer is hidden then; a mouse that moves shows it again. */
int MiniwinTouchMode(void);

/* Called while the game waits for typed text (a name): on a touch screen the on-screen keyboard opens, and closes
 * again half a second after the last call. */
void MiniwinWantText(void);

/* A typed character (ASCII; 8 Backspace, 13 Enter) for the game, as a tap of its key: the web page's text box sends
 * what the phone's keyboard types (its keys don't say which key they are). The game sees one character per two
 * reads of the keyboard (MiniwinTypedKey). */
void MiniwinTypeChar(int ch);

/* The typed character's key (an SDL scancode) down in this read of the keyboard, else 0; *shift = 1 with Shift. */
int MiniwinTypedKey(int *shift);

/* 1 once if the key (an SDL scancode) went down since the last call: a press too short for two reads of the
 * keyboard (an on-screen keyboard's) is still seen. */
int MiniwinKeyLatched(int scancode);
/* 1 while Caps Lock is on. */
int MiniwinCapsLock(void);

/* [library:input] DirectInput on SDL3 (miniwin/src/dinput.c), called like DirectInputCreateA: the system keyboard
 * and mouse devices. Their state is read from SDL (which the game's message loop updates, MiniwinPumpEvents), in
 * DirectInput's layout: the keyboard as 256 bytes by DIK code, the mouse as a DIMOUSESTATE. */
HRESULT WINAPI MiniwinDirectInputCreate(HINSTANCE instance, DWORD version, LPDIRECTINPUTA *out, LPUNKNOWN outer);

/* 1 while the key with this DIK code is down (and the window has the focus). */
int MiniwinKeyDown(int dik);

/* [library:audio] DirectSound on SDL3 audio (miniwin/src/dsound.c), called like DirectSoundCreate: secondary
 * buffers of 8- or 16-bit PCM, mixed on SDL's audio thread with DirectSound's volume, pan, frequency, looping and
 * cursors; duplicates share their data. Fails (DSERR_NODRIVER) when SDL finds no audio device. */
HRESULT WINAPI MiniwinDirectSoundCreate(LPCGUID guid, LPDIRECTSOUND *out, LPUNKNOWN outer);

/* While the window lacks the focus, sounds are muted but keep playing (as DirectSound does); with hold set they
 * hold instead, cursors and all (extensions/ext_movie.h). */
void MiniwinSoundHoldWhenInactive(int hold);

/* Exempts a buffer from that hold: it keeps playing, muted, as with DirectSound (the music, whose synthesizer keeps
 * time by its buffer). */
void MiniwinSoundKeepPlayingInactive(LPDIRECTSOUNDBUFFER buffer);

/* Gives a buffer a source: as the mixer plays it, source writes the bytes just ahead of the play position (into the
 * buffer's own memory, in its format), as DirectMusic's synthesizer streams into its buffer. NULL removes it. Called
 * on the audio thread, under the mixer's lock: it must not call back into DirectSound. */
void MiniwinSoundBufferSetSource(LPDIRECTSOUNDBUFFER buffer, void (*source)(void *ctx, void *bytes, DWORD size),
    void *ctx);

#ifndef _WIN32
/* -port-selftest: the game's fonts and text drawn as the game draws them, the pixels counted (miniwin/src/posix/gdi.c);
 * returns the problems (text that draws nothing) */
int MiniwinTextSelfTest(FILE *report);

/* A line for the port's trace (port/port_posix.c sets it; NULL: none): what miniwin's GDI reports */
extern void (*MiniwinTrace)(const char *line);
#endif

#endif
