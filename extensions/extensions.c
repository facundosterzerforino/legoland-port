/* Optional features: see extensions.h. */
#include <string.h>

#include "ext_console.h"
#include "extensions.h"

int ExtSkipLogo = 1;

void ExtensionsParseCommandLine(const char *cmdline) {
    if (cmdline == NULL) {
        return;
    }
    if (strstr(cmdline, "-no-skip-logo") != NULL) {
        ExtSkipLogo = 0;
    }
    if (strstr(cmdline, "-console") != NULL) {
        ExtConsoleStart();
    }
}
