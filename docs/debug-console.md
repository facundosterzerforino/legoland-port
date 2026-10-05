# Debug console

A text console for playtesting (`extensions/ext_console.c`). It is off by default.

## Turning it on

Add `-console` to the command line, for example in the shortcut's target:

```
"C:\Users\fsterzer\Dropbox\legoland pc port\run\legoland-windowed.exe" WINDEBUG -console
```

(The shortcut's "Start in" folder stays `installed\`.) A second window called "LEGOLAND debug console" opens next to
the game. Type a command and press Enter.

- Use windowed mode with the console. In full screen, clicking the console minimizes the game.
- Commands run as soon as you press Enter, even while the game window is in the background. The game itself
  is paused while its window is in the background, so click back on the game to see the effect.
- Everything the console prints also goes to `run\legoland-port-trace.txt`, prefixed `console:`.
- Ctrl+C in the console does nothing. **Closing the console window closes the game** (Windows doesn't let a
  program refuse that).
- Commands are not case-sensitive. Most of them only work while a level is loaded; they say so otherwise.

## Commands

### Info

| Command | What it does |
|---|---|
| `help` (or `?`) | Lists the commands. |
| `status` | Level, game time, money, visitors, ride wear, whether the level script is running, the inspector's timer (time to the next appraisal) and the appraisal record (passes or failures in a row, and how many failures lose the level). |
| `objectives` (or `obj`) | What the level script is waiting for right now: each objective with its kind (`one-off`, `ongoing`, `PERMANENT`, `REMINDER`), its script keyword (`NEED`, `LINK`, `RANGE`...), the object and number, and whether it is met. For `RANGE` (a theme area) it shows the different kinds of objects needed and placed, and what you have now. Also says when this is the level's last step. |
| `note <text>` | Writes `NOTE: <text>` into the trace, to mark what you were doing when something went wrong. |

### Level flow

| Command | What it does |
|---|---|
| `skip` | Completes the current step: its unmet objectives are dropped, and the game then gives that step's rewards and moves on as if you had done it. Unmet `PERMANENT` objectives from earlier steps still block the script; `skip` tells you if there are any. |
| `skip perm` | The same, and also drops unmet `PERMANENT` objectives (for the rest of the level). |
| `win` | Ends the level now, won (as the original cheat `WELOVELEGOLAND`). |
| `lose` | Ends the level now, lost. |
| `level t1` ... `level t5` | Goes to tutorial 1 to 5 (through the progress screen). |
| `level 1` ... `level 10` | Goes to game level 1 to 10. |
| `appraise` | The inspector comes now. |
| `inspector <minutes>` | Sets the time between appraisals and restarts the timer; `inspector 0` turns the inspector off. (The scripts use 10.) |
| `stopscript` | Stops the level script, as the original cheat `IMPROVISE`. Unless the level is set up otherwise, that ends it as won. |

### Park

| Command | What it does |
|---|---|
| `money <n>` | Adds `n` coins (a negative number takes coins away). |
| `money set <n>` | Sets the coins to `n`. |
| `ridewear <n>` | Ride wear level: `0` = rides never wear out (as the original cheat). The level scripts use 0 to 3. |
| `capacity` | Shows the park's capacity calculations (original cheat). |
| `switches` | Sets map switches 0 to 3 (original cheat; what they do on each map isn't known yet). |

### Other

| Command | What it does |
|---|---|
| `music theme` / `egypt` / `inca` / `castle` / `west` | Switches the music theme. |
| `music stop` | Stops the music. |
| `unlock` | Marks all 15 levels as completed in the current profile. **This is saved in the profile.** |

## The original cheats still work too

Tap Shift once (it types the `:`), then type the word, while a level is running:
`ILIKETOTRAVEL` + `T1`..`T5` / `01`..`09` / `10`, `WELOVELEGOLAND` (win), `PRAISEME` (appraisal now),
`COLDHARDCASH` (+5000 coins), `HARDASNAILS` (no ride wear), `IMPROVISE` (stop the script),
`SHOWCAPACITY`, `DIGGER`.

## Notes for developers

- Commands run on the main thread in `ProcessSystemEvents` (`src/legoland/wndenv.c`), right after the
  original cheat keys are read, and also while it waits for the window to become active again.
- `skip` doesn't free script events (a popup can run the message loop from inside the script runner while it
  walks the list): it turns them into type 0, which has no handler, and the runner frees them when the step
  ends.
- The event type to keyword table in `ext_console.c` comes from the script keyword tables in the exe at
  `0x4bb6f0` and the event builders in `nerps.c`.
