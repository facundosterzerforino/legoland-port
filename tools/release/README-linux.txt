LEGOLAND for Linux
==================
Version @VERSION@

This is a port of LEGO Media's LEGOLAND (1999), rebuilt from the original game's code. It plays the game from your
own LEGOLAND CD: no game files are included. It runs on 64-bit Linux (x86_64) with glibc 2.31 or later: Ubuntu
20.04, Debian 11, Fedora 32 and later, on X11 or Wayland, with PipeWire, PulseAudio or ALSA for the sound.

This is the first Linux release. It passes the port's own checks with the same results as on Windows; it hasn't
been played much on Linux yet: please report anything that misbehaves.


Installing
----------
1. Unpack this folder somewhere you can write to, such as your home folder.
2. Start "legoland" in it (from a file manager, or ./legoland in a terminal).
3. The first time, it asks for the game's files. It finds the LEGOLAND CD by itself when it's mounted, or when its
   image (an .iso, or a .bin) is next to the program; otherwise choose the image, a copy of the CD, or an old
   LEGOLAND installation in the window that opens. The CDs in other languages work too (Spanish, German, Czech...);
   the game plays in the CD's language. The files (about 400 MB) are copied into this folder, once. After that the
   CD isn't needed.


The music
---------
The music plays with GeneralUser GS (GeneralUser-GS.sf2 here), free General MIDI instruments. For the original
sound, copy Windows' own from a Windows PC (C:\Windows\System32\drivers\gm.dls) into this folder: it's used
instead when it's there. (Any other General MIDI SoundFont works too: put its .sf2 here instead of GeneralUser's.)


Playing
-------
The game starts full screen at the desktop's resolution. Command-line switches:
  -windowed, -fullscreen, -res 1024x768   in a window, full screen, or full screen in that mode
  -level-music / -no-level-music          the music follows the level (each one plays its own theme), or not
  -no-pause-movies                        movies keep running in the background
  -no-skip-logo                           the logo at the start can't be skipped
The choice is kept in legoland.ini here ([display]: mode = desktop, change or window; width, height).

Saved games are in the "profiles" folder here. Printing a certificate saves it as a picture in this folder.


Problems
--------
If the game stops with an error, it writes exceptlog.txt and legoland-port-trace.txt here.
Please send them, with what you were doing, to: @REPO@/issues

Source code and licenses: @REPO@ and the "licenses" folder.
LEGOLAND is a trademark of the LEGO Group. This port is not made, endorsed or supported by the LEGO Group.
