#ifndef PORT_LANGUAGE_H
#define PORT_LANGUAGE_H

/* [port] The language of the game's CD, as the European CDs' build of the game knows it.
 *
 * The English CD's build (the one the decomp matches) has one language. The European CDs (Spanish, Italian, ...)
 * carry a different build and data: every object class holds its name and texts in nine languages (english,
 * german, danish, french, italian, spanish, swedish, norwegian, dutch), and strings\stab.str has string 9999,
 * the CD's language ("SPANISH"). That build picks the object texts by it, sets the 18-point font's weight to 500
 * for German and Dutch (600 otherwise), and places the tutorial screen's entries at x 30 in English, 10 in the
 * other languages. The port does the same when string 9999 is there, so either CD's data plays as it did. */

/* The CD's language (string 9999, e.g. "SPANISH"), or NULL for the English CD (no such string). */
const char *PortGameLanguage(void);

/* Non-zero when the CD's language is name (any case). */
int PortGameLanguageIs(const char *name);

#endif
