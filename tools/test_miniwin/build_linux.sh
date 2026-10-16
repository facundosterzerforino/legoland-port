#!/bin/sh
# Builds the miniwin tests that run off Windows (miniwin/src/posix/), with the host's clang and the SDL3 the linux
# preset fetched (build it first): test_seh (structured exception handling on signals), test_gdi_print (the picture
# printer), test_setup (the first-run setup from a CD, its image or an install). Into build-linux/tests/.
set -e
cd "$(dirname "$0")"
P=$(pwd)/../..
SDLB=$P/build-linux/_deps
OUT=$P/build-linux/tests
mkdir -p "$OUT"
CFLAGS="-O2 -fms-extensions -Wno-microsoft-anon-tag -D_GNU_SOURCE -I$P/miniwin/include/win32 -I$P/miniwin/include \
    -I$P/miniwin/src -I$P/port -I$SDLB/sdl3-src/include -I$SDLB/sdl3-build/include-revision -I$SDLB/sdl3-build/include"
LIBS="$SDLB/sdl3-build/libSDL3.a -lm -lpthread -ldl"
POSIX="$P/miniwin/src/posix/*.c $P/miniwin/src/files.c $P/miniwin/src/window.c"
clang $CFLAGS test_seh.c $P/miniwin/src/posix/seh.c -lpthread -o "$OUT/test_seh"
clang $CFLAGS test_gdi_print.c $POSIX $LIBS -o "$OUT/test_gdi_print"
clang $CFLAGS test_setup.c $P/port/port_setup.c $P/port/blast/blast.c $P/port/puff/puff.c $LIBS -o "$OUT/test_setup"
echo "built in $OUT: test_seh test_gdi_print test_setup"
