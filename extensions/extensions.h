#ifndef EXTENSIONS_H
#define EXTENSIONS_H

/* Optional features that change how the game behaves (see PORTING.md, "extensions/"). Each has a
 * command-line switch. */

/* A click or key skips the LEGO Media logo movie at startup (the original only skips it with Ctrl+Q).
 * On by default at Facu's request (2026-10-03); -no-skip-logo turns it off. */
extern int ExtSkipLogo;

/* -console opens a debug console for playtesting (ext_console.h; commands in docs/debug-console.md). */

/* Level music (ext_music.h): each level plays the music theme that fits it. Off by default; the launcher's "Music
 * changes with the level" box saves the choice, and -level-music / -no-level-music override it. */

/* Pause movies (ext_movie.h): a movie stops while the game is in the background instead of running on. On by
 * default (the maintainer's choice); -no-pause-movies turns it off. */

/* Portable music (-portable-music, off for now): the music plays through miniwin's DirectMusic
 * (miniwin/dmusic.h) instead of Windows', to compare the two; it becomes the default once it sounds right. */

/* Reads the switches from the command line (after the launcher, so they override its saved choices). */
void ExtensionsParseCommandLine(const char *cmdline);

/* The extensions' per-frame work (console commands, level music). Main thread, where the original reads its cheat
 * keys (ProcessSystemEvents). */
void ExtensionsPoll(void);

#endif
