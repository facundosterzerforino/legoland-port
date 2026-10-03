#ifndef EXTENSIONS_H
#define EXTENSIONS_H

/* Optional features that change how the game behaves (see PORTING.md, "extensions/"). Each one is off unless
 * its switch is on the command line. */

/* -skip-logo: a click or key skips the LEGO Media logo movie at startup (the original only skips it with
 * Ctrl+Q). */
extern int ExtSkipLogo;

/* Reads the switches from the command line. */
void ExtensionsParseCommandLine(const char *cmdline);

#endif
