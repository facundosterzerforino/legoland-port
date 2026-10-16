LEGOLAND for the web
====================
Version @VERSION@

This is a port of LEGO Media's LEGOLAND (1999), rebuilt from the original game's code, for web browsers
(WebAssembly). It plays the game from your own LEGOLAND CD: no game files are included. It runs in recent Chrome,
Edge, Firefox and Safari (17 or later) on a computer.

This is the first web release. It passes the port's own checks in a browser with the same results as on Windows;
it hasn't been played in a browser yet: please report anything that misbehaves.


Putting it online
-----------------
This folder is a website: put its files on any web server (GitHub Pages, Netlify, your own...), served over HTTPS,
and open index.html. To try it on your own computer: in this folder, run
    python3 -m http.server 8000
and open http://localhost:8000/ in the browser.

The game uses threads, for which the page must be "cross-origin isolated": coi-serviceworker.js arranges that by
itself (the page reloads once on the first visit), so no server settings are needed.


Playing
-------
1. The first time, the page asks for the game's files: choose your LEGOLAND CD's image (an .iso, or a .bin; any
   language's CD). It's copied into this browser's storage for the site (nothing is uploaded), and the game
   installs from it when it starts (about 400 MB, kept for your next visits).
2. The music plays with GeneralUser GS (GeneralUser-GS.sf2 here), free General MIDI instruments the page stores
   with the game. For the original sound, add Windows' own with the page's "Add Windows' gm.dls..." button (it's in
   C:\Windows\System32\drivers on any Windows PC).
3. Click Play. The game goes full screen; Esc leaves full screen.
Saved games stay in the browser's storage for the site; "Remove the game's files" deletes them with the game.
Printing a certificate saves it as a picture in that storage too.


Problems
--------
Please report problems, with what you were doing and the browser's name, to: @REPO@/issues

Source code and licenses: @REPO@ and the "licenses" folder.
LEGOLAND is a trademark of the LEGO Group. This port is not made, endorsed or supported by the LEGO Group.
