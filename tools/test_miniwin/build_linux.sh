#!/bin/sh
# Builds the miniwin tests that run off Windows (miniwin/src/posix/), with the host's clang and the SDL3 the linux
# preset fetched (build it first): test_seh (structured exception handling on signals), test_gdi_print (the picture
# printer), test_touch (taps, typing and the on-screen keyboard on touch screens), test_present (the picture on the screen), test_timer (the clock waits), test_readcache (the read cache), test_setup (the first-run setup from a CD, its image or an install), test_dmusic (the game's music with
# miniwin's DirectMusic and FluidSynth). Into build-linux/tests/.
set -e
cd "$(dirname "$0")"
P=$(pwd)/../..
SDLB=$P/build-linux/_deps
OUT=$P/build-linux/tests
mkdir -p "$OUT"
WIDE="-fno-builtin-wcslen -fno-builtin-wcscpy -fno-builtin-wcsncpy -fno-builtin-wcscat -fno-builtin-wcscmp -fno-builtin-wcschr -fno-builtin-wcsrchr -fno-builtin-wmemcpy -fno-builtin-wmemmove -fno-builtin-wmemset -fno-builtin-wmemcmp -fno-builtin-wmemchr"
CFLAGS="-O2 -fno-strict-aliasing $WIDE -fms-extensions -Wno-microsoft-anon-tag -D_GNU_SOURCE -I$P/miniwin/include/win32 -I$P/miniwin/include \
    -I$P/miniwin/src -I$P/port -I$SDLB/sdl3-src/include -I$SDLB/sdl3-build/include-revision -I$SDLB/sdl3-build/include"
LIBS="$SDLB/sdl3-build/libSDL3.a -lm -lpthread -ldl"
POSIX="$P/miniwin/src/posix/*.c $P/miniwin/src/files.c $P/miniwin/src/window.c"
clang $CFLAGS test_seh.c $P/miniwin/src/posix/seh.c -lpthread -o "$OUT/test_seh"
clang $CFLAGS test_gdi_print.c $POSIX $LIBS -o "$OUT/test_gdi_print"
clang $CFLAGS test_touch.c $POSIX $LIBS -o "$OUT/test_touch"
clang $CFLAGS test_present.c $POSIX $LIBS -o "$OUT/test_present"
clang $CFLAGS test_timer.c $P/miniwin/src/timer.c $LIBS -o "$OUT/test_timer"
clang $CFLAGS test_readcache.c $POSIX $LIBS -o "$OUT/test_readcache"
clang $CFLAGS test_setup.c $P/port/port_setup.c $P/port/blast/blast.c $P/port/puff/puff.c $POSIX $LIBS -o "$OUT/test_setup"
DM=$P/port/dmusic
# as the game's build: _GNU_SOURCE for the POSIX layer only (with it, glibc's <stdlib.h> declares C11's once_flag,
# which dmusic's thread shim, included by miniwin's DirectMusic, declares too)
T=$OUT/dmusic-objects
mkdir -p $T
for f in $P/miniwin/src/posix/*.c $P/miniwin/src/files.c $P/miniwin/src/window.c; do
    clang $CFLAGS -fshort-wchar -c $f -o $T/$(basename $f .c).o
done
for f in test_dmusic.c $P/miniwin/src/dsound.c $P/miniwin/src/dmusic.c $P/miniwin/src/dmengine.c $P/miniwin/src/dmreverb.c; do
    clang $(echo $CFLAGS | sed 's/-D_GNU_SOURCE//') -fshort-wchar -DDM_STATIC=1 -DDME_FLUIDSYNTH -DWITH_FLUIDSYNTH \
        -I$DM/include -I$DM/src -I$DM/vendor/TinySoundFont -I$SDLB/fluidsynth-build/include \
        -I$SDLB/fluidsynth-src/include -I$P/tools -c $f -o $T/$(basename $f .c).o
done
clang $T/*.o $P/build-linux/libdmusic.a $SDLB/fluidsynth-build/src/libfluidsynth.a $LIBS -lstdc++ -o "$OUT/test_dmusic"
echo "built in $OUT: test_seh test_gdi_print test_touch test_present test_timer test_readcache test_setup test_dmusic"
