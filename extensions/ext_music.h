#ifndef EXT_MUSIC_H
#define EXT_MUSIC_H

/* Level music (off by default; the launcher's "Music follows the level" box, or -level-music /
 * -no-level-music). The game has five music themes (theme, egypt, inca, castle, west) but the original only ever
 * plays "theme"; the other four are reachable only with cheat codes. With this on, each level plays the theme that
 * fits it (the table in ext_music.c), and the menus go back to "theme". */

extern int ExtLevelMusic;

/* Switches the music when a level starts or the game goes back to the menus. Main thread, once per frame. */
void ExtLevelMusicPoll(void);

/* The music theme a level plays with level music on (0 theme, 1 egypt, 2 inca, 3 castle, 4 west). */
int ExtLevelMusicTheme(unsigned int level);

#endif
