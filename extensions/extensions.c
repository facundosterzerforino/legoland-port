/* Optional features: see extensions.h. */
#include <string.h>

#include "extensions.h"

int ExtSkipLogo = 1;

void ExtensionsParseCommandLine(const char *cmdline) {
    if (cmdline == NULL) {
        return;
    }
    if (strstr(cmdline, "-no-skip-logo") != NULL) {
        ExtSkipLogo = 0;
    }
}
