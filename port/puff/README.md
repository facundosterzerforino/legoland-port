# puff

`puff.c` and `puff.h` are zlib's `contrib/puff` (Mark Adler, version 2.3), unchanged from zlib 1.3.1
(https://github.com/madler/zlib/tree/v1.3.1/contrib/puff), under the zlib license in their headers.

[library:filesystem] They decompress raw Deflate, which the Czech LEGOLAND CD's `data\lego.pak` (a "CPack 1.3"
archive by Centauri Production, in place of the other CDs' `main.z`) is packed with, for the first-run setup
(`port/port_setup.c`).
