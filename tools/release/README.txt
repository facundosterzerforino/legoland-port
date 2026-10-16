LEGOLAND for Windows 11
=======================
Version @VERSION@

This is a port of LEGO Media's LEGOLAND (1999) to modern Windows, rebuilt from the original game's code. It plays
the game from your own LEGOLAND CD: no game files are included. It is tested on Windows 11 and should run on
Windows 10 too.


Installing
----------
1. Unzip this folder somewhere you can write to, such as Documents or the Desktop (not Program Files).
2. Start LEGOLAND-port.exe. (Don't rename it to LEGOLAND.exe: Windows would treat it as the 1999 game
   and apply its old compatibility fixes, and it wouldn't start.)
3. The first time, it asks for the game's files. Insert your LEGOLAND CD, or click "Choose ISO..." and pick
   your ISO image of it (no need to mount it), and click Install. The window speaks your Windows' language
   (English, Spanish, German, French, Italian, Dutch, Danish, Swedish, Norwegian or Czech); change it with the
   Language list. The CDs in other languages work too (the Czech CD included); the game plays in the CD's
   language. You can also choose a folder: a copy of the
   CD, or an old LEGOLAND installation (its saved games come along).
   The files (about 400 MB) are copied into this folder, once. After that the CD isn't needed.


Playing
-------
Before the game starts, a small window asks how to show it:
- Full screen, keeping the desktop's resolution (the default);
- Full screen, changing the resolution;
- In a window, of a size you choose.
Tick "Don't show this again" to skip it; hold Shift while starting the game to see it again.

Saved games are in the "profiles" folder here.


New in this version
-------------------
- "Music follows the level" (a box in that window): each level plays the music theme that fits it. The original
  only ever plays the main theme.
- Movies pause while the game is in the background, and carry on where they were when you come back.
- The music plays through the port's own synthesizer (FluidSynth, on Windows' General MIDI instruments).
- A click skips the LEGO Media logo at startup.

Command-line switches (add them to a shortcut's target):
  -windowed, -fullscreen, -res 1024x768   choose without the window above; -launcher always shows it
  -level-music / -no-level-music          music follows the level, or not
  -no-pause-movies                        movies keep running in the background, as in the original
  -windows-music                          play the music through Windows' DirectMusic instead
  -no-skip-logo                           the logo can't be skipped


Problems
--------
If the game stops with an error or freezes, it writes a report next to LEGOLAND-port.exe (crash-*.dmp, freeze-*.dmp,
exceptlog.txt, legoland-port-trace.txt and legoland-port-trace-prev.txt).
Please send them, with what you were doing, to: @REPO@/issues

Source code and licenses: @REPO@ and the "licenses" folder.
LEGOLAND is a trademark of the LEGO Group. This port is not made, endorsed or supported by the LEGO Group.
