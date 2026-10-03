/* Optional features, off by default: see extensions.h. */
#include <string.h>

#include "extensions.h"

int ExtSkipLogo;

void ExtensionsParseCommandLine(const char *cmdline) {
    if (cmdline == NULL) {
        return;
    }
    ExtSkipLogo = strstr(cmdline, "-skip-logo") != NULL;
}
