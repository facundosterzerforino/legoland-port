#ifndef PORT_WATCHDOG_H
#define PORT_WATCHDOG_H

/* [port] Freeze and crash reporting, port-only.
 *
 * A background thread watches a heartbeat that the game's main thread bumps (ProcessSystemEvents, DebugTrace,
 * DrawWatchSprite). If it stops for PORT_WATCHDOG_FREEZE_MS while the game isn't paused on purpose, the thread
 * writes "FREEZE detected" to legoland-port-trace.txt with the main thread's call stack (symbolized from
 * legoland.pdb when it sits next to the exe, plus RVAs for llvm-symbolizer) and saves a minidump
 * (freeze-<time>.dmp). A first-chance access violation or stack overflow inside the exe is reported the same way
 * (crash-<time>.dmp) before the game's own handler runs, since that handler can't run on a broken stack. */

#define PORT_WATCHDOG_FREEZE_MS 3000

void PortWatchdogStart(void);
void PortHeartbeat(void);
void PortSetPhase(const char *phase); /* a string literal or other static text; also bumps the heartbeat */
void PortSetPaused(int paused); /* the game waits for its window on purpose: don't report */

#endif
