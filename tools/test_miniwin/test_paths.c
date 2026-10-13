/* test_paths: checks miniwin's path translation (miniwin/src/files.c) the way Linux and Android will need it. It
 * makes a case-sensitive folder in Windows' temp folder (Windows can mark one: fsutil file setCaseSensitiveInfo, no
 * admin needed; not on WSL's own disk, where the repo is), puts files in it under one spelling, asks for them the way
 * the game does (backslashes, other case), and removes the folder again.
 *
 *   tools/test_miniwin/build.sh && tools/test_miniwin/test_paths.exe      (prints FAIL lines, exits 1 on any) */
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "miniwin/files.h"

#define ROOT "legoland-paths-test"

static int fails;

static void Check(const char *in, const char *want) {
    char buf[MAX_PATH];
    const char *got = MiniwinPath(in, buf, sizeof(buf));
    int ok = strcmp(got, want) == 0;

    printf("%-4s %-52s -> %s%s%s\n", ok ? "ok" : "FAIL", in, got, ok ? "" : "   want ", ok ? "" : want);
    fails += !ok;
}

static void Touch(const char *path) {
    FILE *f = fopen(path, "w");
    if (f != NULL) {
        fputs("x", f);
        fclose(f);
    }
}

int main(void) {
    char cwd[MAX_PATH];
    char command[MAX_PATH + 64];
    FILE *f;

    if (GetTempPathA(sizeof(cwd), cwd) == 0 || _chdir(cwd) != 0) {
        printf("FAIL: no temp folder\n");
        return 1;
    }
    system("rmdir /s /q " ROOT " 2>nul");
    if (_mkdir(ROOT) != 0 || _getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("FAIL: can't make %s\n", ROOT);
        return 1;
    }
    sprintf(command, "fsutil file setCaseSensitiveInfo \"%s\\" ROOT "\" enable >nul", cwd);
    if (system(command) != 0) {
        printf("FAIL: can't make %s case-sensitive (fsutil needs the Windows Subsystem for Linux feature)\n", ROOT);
        return 1;
    }
    _chdir(ROOT); /* new folders inside inherit the flag */
    _mkdir("volumes");
    _mkdir("Speech");
    _mkdir("rollercoaster");
    _mkdir("rollercoaster/RollerCoaster");
    _mkdir("rollercoaster/RollerCoaster/createddata");
    _mkdir("Profiles");
    Touch("volumes/Legoland.res");
    Touch("Speech/a01.WAV");
    Touch("rollercoaster/RollerCoaster/createddata/Track.dat");
    f = fopen("VOLUMES\\LEGOLAND.RES", "rb");
    if (f != NULL) {
        fclose(f);
        printf("FAIL: the folder isn't case-sensitive, nothing would be tested\n");
        fails++;
    }

    Check(".\\volumes\\Legoland.res", "./volumes/Legoland.res");
    Check(".\\VOLUMES\\legoland.RES", "./volumes/Legoland.res");
    Check("speech\\A01.wav", "Speech/a01.WAV");
    Check("RollerCoaster\\RollerCoaster\\CreatedData\\track.dat", "rollercoaster/RollerCoaster/createddata/Track.dat");
    Check("profiles\\new.txt", "Profiles/new.txt"); /* a file to be created: the folder is matched */
    Check("profiles\\*.*", "Profiles/*.*");
    Check("nothere\\file.txt", "nothere/file.txt");
    Check("\\\\.\\vwin32", "\\\\.\\vwin32");
    f = MiniwinFopen("VOLUMES\\LEGOLAND.RES", "rb");
    printf("%-4s MiniwinFopen of a path in another case\n", f != NULL ? "ok" : "FAIL");
    fails += f == NULL;
    if (f != NULL) {
        fclose(f);
    }

    _chdir("..");
    system("rmdir /s /q " ROOT);
    printf("%d failed\n", fails);
    return fails != 0;
}
