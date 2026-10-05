#ifndef EXT_CONSOLE_H
#define EXT_CONSOLE_H

/* Debug console for playtesting (switch -console, off by default). Opens a text console next to the game;
 * commands typed there run on the game's main thread, at the point where the original cheat keys are read
 * (ProcessSystemEvents), so they are as safe as the original cheats. "help" lists the commands. */

extern int ExtConsole;

/* Opens the console window and starts reading commands (called once, when -console is given). */
void ExtConsoleStart(void);

/* Runs the commands typed since the last call. Main thread only. */
void ExtConsolePoll(void);

#endif
