#ifndef EXT_ACTIONS_H
#define EXT_ACTIONS_H

#include <stdarg.h>

/* Action log: what the player does and what the game does in answer, as "action:" lines in the trace
 * (legoland-port-trace.txt, and the browser's console on the web), so a bug report's log shows the steps that led
 * to it. On by default at the maintainer's request (2026-10-17); -no-action-log turns it off.
 *
 * Every frame (ExtActionsPoll): mouse clicks with what is under the cursor (a button and its tooltip, an object, a
 * visitor or worker, the ground) and drags, keys, and changes of the screen (park, map, menus), the mode (query,
 * build, erase, path), the build menu and its tab, the game timer's pause, the level. From the game's code: the
 * advisor's messages, objects built and removed, workers hired, picked up and dropped, info popups, saves and
 * loads, the end of a level, and the game's own debug messages (DBPrintf, compiled out of the original) except the
 * ones it prints every frame. At most 40 lines a second; the rest are counted. */
extern int ExtActionLog;

/* One "action:" line (printf format). */
void ExtAction(const char *fmt, ...);

/* The game's DBPrintf: a "game:" line, unless it is one of the messages printed every frame. */
void ExtActionGameMessage(const char *format, va_list ap);

/* What a hover type and pointer (struct HoverInfo) are, in words: "object "SPIDER RIDE"", "a mechanic", ... (a
 * static buffer, until the next call). */
const char *ExtActionHover(int type, const void *ptr);

/* The per-frame watch (ExtensionsPoll, after the input is read). */
void ExtActionsPoll(void);

#endif
