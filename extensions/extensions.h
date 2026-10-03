#ifndef EXTENSIONS_H
#define EXTENSIONS_H

/* Optional features that change how the game behaves (see PORTING.md, "extensions/"). Each has a
 * command-line switch. */

/* A click or key skips the LEGO Media logo movie at startup (the original only skips it with Ctrl+Q).
 * On by default at Facu's request (2026-10-03); -no-skip-logo turns it off. */
extern int ExtSkipLogo;

/* Reads the switches from the command line. */
void ExtensionsParseCommandLine(const char *cmdline);

#endif
