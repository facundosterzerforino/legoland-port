/* Optional features: see extensions.h. */
#include <string.h>

#include "ext_actions.h"
#include "ext_console.h"
#include "ext_movie.h"
#include "ext_music.h"
#include "extensions.h"
#include "miniwin/dmusic.h"

int ExtSkipLogo = 1;

void ExtensionsParseCommandLine(const char *cmdline) {
    if (cmdline == NULL) {
        return;
    }
    if (strstr(cmdline, "-no-skip-logo") != NULL) {
        ExtSkipLogo = 0;
    }
    if (strstr(cmdline, "-no-level-music") != NULL) {
        ExtLevelMusic = 0;
    } else if (strstr(cmdline, "-level-music") != NULL) {
        ExtLevelMusic = 1;
    }
    if (strstr(cmdline, "-no-pause-movies") != NULL) {
        ExtPauseMovies = 0;
    }
    ExtPauseMoviesApply();
    if (strstr(cmdline, "-windows-music") != NULL) {
        MiniwinUsePortableMusic(0);
    } else if (strstr(cmdline, "-portable-music") != NULL) {
        MiniwinUsePortableMusic(1);
    }
    if (strstr(cmdline, "-no-action-log") != NULL) {
        ExtActionLog = 0;
    }
    if (strstr(cmdline, "-console") != NULL) {
        ExtConsoleStart();
    }
}

void ExtensionsPoll(void) {
    ExtActionsPoll();
    ExtConsolePoll();
    ExtLevelMusicPoll();
}
