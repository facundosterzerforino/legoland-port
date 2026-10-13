#ifndef EXT_MOVIE_H
#define EXT_MOVIE_H

/* Pause movies (on by default; -no-pause-movies turns it off, to compare with the original). The original keeps a movie's clock running while the game waits for its window (Alt+Tab, a
 * click elsewhere): back in the game, the movie jumps to where the clock says, and what it "played" meanwhile is
 * skipped. With this on, the time the game waited is taken off the movie's clock (PlayMovieFrames), and the game's
 * sounds hold instead of only going quiet while the window lacks the focus, so the movie goes on where it was,
 * picture and sound together.
 *
 * It also fixes the original's catch-up after a jump the pause can't prevent (a stall longer than the sound buffer,
 * about 1.5 s, such as a display mode change). UpdateAviAudioBuffer then refills the buffer from the movie's new
 * position, but gets two units wrong: it restarts playback at a position in samples where DirectSound expects bytes
 * (for 16-bit stereo, a quarter of the way to the right place, so stale sound plays and the buffer stays out of
 * step), and for compressed sound (most of the movies, the intro too) it seeks in samples where the AVI counts
 * blocks, which lands past the end: the rest of the movie is silent. */

extern int ExtPauseMovies;

/* Applies the choice (the sound mixer's behaviour). After the command line. */
void ExtPauseMoviesApply(void);

/* ProcessSystemEvents: waiting is 1 while it waits for the game's window, 0 once it goes on. */
void ExtPauseMoviesWaiting(int waiting);

/* The milliseconds the game waited for its window since the last call (always 0 when the extension is off). */
int ExtPauseMoviesTake(void);

/* UpdateAviAudioBuffer's catch-up to frame: sets the sound's read position and returns the position to restart
 * playback at (play_pos, the original's, when the extension is off). */
int ExtPauseMoviesResync(unsigned int frame, int play_pos);

#endif
