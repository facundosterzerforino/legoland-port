/* test_setup: builds a data folder with the port's setup (port/port_setup.c) from a LEGOLAND CD (a drive, a mounted
 * ISO or a copy of one), an ISO image file read directly, or an existing install, printing the progress; then
 * compare the result with an install (tools: diff -r).
 *
 *   tools/test_miniwin/test_setup.exe <source folder or .iso> <target folder> */
#include <SDL3/SDL.h>
#include <stdio.h>

#include "port_setup.h"

static int last = -1;

static void Report(void *ctx, unsigned long long done, unsigned long long total, const char *file) {
    int pct = total ? (int)(done * 100 / total) : 100;

    (void)ctx;
    if (pct / 10 != last / 10) {
        printf("%3d%%  %s\n", pct, file);
        fflush(stdout);
        last = pct;
    }
}

int main(int argc, char **argv) {
    PortSetupProgress progress = {Report, NULL, NULL};
    char error[512];
    static const char *const kinds[] = {"not usable", "a CD", "an install", "a CD image"};
    int kind;

    if (argc < 3) {
        printf("usage: test_setup <source folder or .iso> <target folder>\n");
        return 2;
    }
    kind = PortSetupKind(argv[1]);
    printf("%s is %s\n", argv[1], kinds[kind]);
    if (!PortSetupInstall(argv[1], argv[2], &progress, error, sizeof(error))) {
        printf("FAILED: %s\n", error);
        return 1;
    }
    printf("OK\n");
    return 0;
}
