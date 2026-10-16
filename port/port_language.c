/* [port] The language of the game's CD. See port_language.h. */
#include "port_language.h"

#include <string.h>

#include "../src/legoland/string.h"

const char *PortGameLanguage(void) {
    const char *lang = GetString(9999);

    return lang != NULL && lang[0] != '\0' ? lang : NULL;
}

int PortGameLanguageIs(const char *name) {
    const char *lang = PortGameLanguage();

    return lang != NULL && _stricmp(lang, name) == 0;
}
