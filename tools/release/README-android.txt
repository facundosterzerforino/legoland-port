LEGOLAND for Android
====================
Version @VERSION@

This is a port of LEGO Media's LEGOLAND (1999), rebuilt from the original game's code. It plays the game from your
own LEGOLAND CD: no game files are included. It runs on Android 7.0 and later, on phones, tablets and Chromebooks
(ARM 64-bit, or x86 64-bit), in landscape.

This is the first Android release. It passes the port's own checks with the same results as on Windows (on an
emulator); it hasn't been played on a device yet: please report anything that misbehaves.


Installing
----------
1. Copy the .apk to the device and open it; allow installing apps from that source when Android asks.
2. Copy your LEGOLAND CD's image (an .iso, or a .bin) to the device too (to Downloads, say).
3. Start LEGOLAND. The first time, it asks for the game's files: choose the CD image. They're installed (about
   400 MB) into the app's folder, once; after that the image can be deleted.
   Instead of the image, you can copy an installed LEGOLAND folder's contents into the app's folder yourself,
   with a file manager or from a computer over USB:
     Android/data/io.github.facundosterzerforino.legolandport/files/


The music
---------
The music plays with GeneralUser GS, free General MIDI instruments the app brings. For the original sound, copy
Windows' own from a Windows PC (C:\Windows\System32\drivers\gm.dls) into the app's folder above: it's used
instead when it's there.


Playing
-------
- One finger is the mouse: touch to click, drag to move. The game's pointer is hidden while you use touch (a
  mouse brings it back).
- Drag the object lists with a finger to scroll them (a tap still picks an object).
- Two fingers moving move around the park; a two-finger tap is the right mouse button (held still: held down).
- The keyboard opens by itself when the game asks for a name; three fingers show or hide it too.
- The back key or gesture is Escape.
Saved games stay in the app's folder (profiles). Printing a certificate saves it as a picture there.


Problems
--------
If the game stops with an error, it writes exceptlog.txt and legoland-port-trace.txt in the app's folder.
Please send them, with what you were doing, to: @REPO@/issues

Source code: @REPO@
LEGOLAND is a trademark of the LEGO Group. This port is not made, endorsed or supported by the LEGO Group.
