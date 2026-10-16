/* [port] The language of the game's CD. See port_language.h. */
#include "port_language.h"

#include <string.h>

#include "../src/legoland/string.h"

const char *PortGameLanguage(void) {
    const char *lang = GetString(9999);

    return lang != NULL && lang[0] != '\0' ? lang : NULL;
}

int PortGameCharset(int charset) {
    static const char *const central[] = {"CZECH", "SLOVAK", "POLISH", "HUNGARIAN", "SLOVENIAN", "CROATIAN"};
    size_t i;

    for (i = 0; i < sizeof(central) / sizeof(central[0]); i++) {
        if (PortGameLanguageIs(central[i])) {
            return 238; /* EASTEUROPE_CHARSET */
        }
    }
    if (PortGameLanguageIs("RUSSIAN")) {
        return 204; /* RUSSIAN_CHARSET */
    }
    if (PortGameLanguageIs("GREEK")) {
        return 161; /* GREEK_CHARSET */
    }
    return charset;
}

int PortGameLanguageIs(const char *name) {
    const char *lang = PortGameLanguage();

    return lang != NULL && _stricmp(lang, name) == 0;
}
