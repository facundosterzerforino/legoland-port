#!/bin/sh
# Builds the music tools (clang-cl + lld-link with the xwin SDK, like the clang-cl-x86 preset). record_dmusic links
# miniwin's DirectSound and the SDL3 that the port build fetched: build build-clang-x86 first. The others build
# port/dmusic in (and play_engine miniwin's style engine).
set -e
cd "$(dirname "$0")"
X="${XWIN_DIR:-$HOME/xwin}"
SDL=../../build-clang-x86/_deps
DM=../../port/dmusic
CFLAGS="/nologo -fuse-ld=lld --target=i686-pc-windows-msvc -fms-compatibility /O2 -MT -D_CRT_SECURE_NO_WARNINGS"
INCLUDES="/imsvc $X/crt/include /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared"
LIBPATHS="/libpath:$X/crt/lib/x86 /libpath:$X/sdk/lib/um/x86 /libpath:$X/sdk/lib/ucrt/x86"
SDLLIBS="$SDL/sdl3-build/SDL3-static.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib \
    oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib"
DMFLAGS="-std:c11 -DDM_STATIC=1 -DDM_BUILD=1 -I$DM/include -I$DM/src -I$DM/vendor/TinySoundFont"
DMSOURCES="$DM/src/*.c $DM/src/io/*.c $DM/src/util/*.c $DM/src/thread/*.c $DM/vendor/TinySoundFont/tsf.c"
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include -I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision \
    record_dmusic.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c /Ferecord_dmusic.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS
for tool in check_notes check_bars; do
    clang-cl $CFLAGS $INCLUDES $DMFLAGS $tool.c $DMSOURCES /Fe$tool.exe /link /machine:x86 $LIBPATHS
done
clang-cl $CFLAGS $INCLUDES $DMFLAGS -I../../miniwin/include play_engine.c ../../miniwin/src/dmengine.c $DMSOURCES \
    /Feplay_engine.exe /link /machine:x86 $LIBPATHS
rm -f *.obj
