# blast

`blast.c` and `blast.h` are zlib's `contrib/blast` (Mark Adler, version 1.3), unchanged from zlib 1.3.1
(https://github.com/madler/zlib/tree/v1.3.1/contrib/blast), under the zlib license in their headers.

[library:filesystem] They decompress the PKWARE Data Compression Library format the LEGOLAND CD's `main.z` (an
InstallShield 3 archive) is packed with, for the first-run setup (`port/port_setup.c`).
