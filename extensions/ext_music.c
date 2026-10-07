/* Level music: see ext_music.h. */
#include <windows.h>

#include "ext_music.h"

#include "../src/legoland/debug.h"
#include "../src/legoland/globals.h"
#include "../src/legoland/sound_sfx.h"

int ExtLevelMusic = 0;

enum {
    MUSIC_THEME,
    MUSIC_EGYPT,
    MUSIC_INCA,
    MUSIC_CASTLE,
    MUSIC_WEST
};

/* lpConfig->level: 1-5 the tutorials, 6-15 game levels 1-10. Every level is a park with several themes, so each gets
 * the theme its story is about (from its objlist<n>.txt). */
static const unsigned char LevelThemes[16] = {
    MUSIC_THEME, /* 0: none */
    MUSIC_THEME, /* tutorial 1: first rides (Space Tower) */
    MUSIC_THEME, /* tutorial 2: gardeners and flowers */
    MUSIC_THEME, /* tutorial 3: path scenery */
    MUSIC_THEME, /* tutorial 4: mechanics and repairs */
    MUSIC_WEST, /* tutorial 5: the first Western objects */
    MUSIC_WEST, /* game level 1: "LEGOLAND and Western" */
    MUSIC_CASTLE, /* game level 2: Castle attractions appear */
    MUSIC_THEME, /* game level 3: the hedge puzzle */
    MUSIC_CASTLE, /* game level 4: Castle is the biggest ask */
    MUSIC_WEST, /* game level 5: the Log Flume round the park */
    MUSIC_WEST, /* game level 6: mostly Western */
    MUSIC_EGYPT, /* game level 7: the Adventurers theme arrives (Egypt movie, Explorer's Institute) */
    MUSIC_THEME, /* game level 8: the right rides in the right place */
    MUSIC_INCA, /* game level 9: the lost Temple and the Jungle Cruise */
    MUSIC_EGYPT, /* game level 10: dinosaurs, Abu Simbel and the pyramids */
};

static const char *const ThemeNames[] = {"theme", "egypt", "inca", "castle", "west"};

/* the theme this extension last asked for; -1 = none yet */
static int Applied = -1;

int ExtLevelMusicTheme(unsigned int level) {
    return level < sizeof(LevelThemes) ? LevelThemes[level] : MUSIC_THEME; /* free play: theme */
}

void ExtLevelMusicPoll(void) {
    int want;

    if (!ExtLevelMusic || MusicEnabled == 0 || DMusicInitialised == 0) {
        return;
    }
    /* EditMode.unk4: 3 in a level, 2 the menus and progress screens; 1 and 0 are the in-game menus, which keep the
     * level's music */
    if (EditMode.unk4 == 3 && MapLoaded != 0) {
        want = ExtLevelMusicTheme(lpConfig->level);
    } else if (EditMode.unk4 == 2) {
        want = MUSIC_THEME;
    } else {
        return;
    }
    if (want == Applied) {
        return;
    }
    if (MusicState == 0) {
        return; /* the music hasn't started yet (it starts after the intro movie) */
    }
    if (MusicState == 1) {
        /* stopped while a movie plays: SetInteractiveMusicTheme would start it now, so only choose the theme the
         * game resumes with afterwards (FUN_00492da0 plays CurrentMusicTheme) */
        CurrentMusicTheme = want;
    } else {
        SetInteractiveMusicTheme(want);
    }
    Applied = want;
    DebugTrace("level music: %s (level %u)", ThemeNames[want], EditMode.unk4 == 3 ? lpConfig->level : 0u);
}
