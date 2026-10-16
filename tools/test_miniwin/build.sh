#!/bin/sh
# Builds the miniwin tests (clang-cl + lld-link with the xwin SDK, like the clang-cl-x86 preset): test_miniwin,
# test_paths, test_dsound, test_acm, test_avifile, test_dmusic, capture_dmusic, instruments_dmusic, test_cinepak, test_charset, test_setup_texts and test_setup. Most link the
# SDL3 that the port build fetched, and the last three port/dmusic and the FluidSynth it built, as the game does:
# build the build-clang-x86 preset first.
set -e
cd "$(dirname "$0")"
X="${XWIN_DIR:-$HOME/xwin}"
SDL=../../build-clang-x86/_deps
DM=../../port/dmusic
DMSOURCES="$DM/src/*.c $DM/src/io/*.c $DM/src/util/*.c $DM/src/thread/*.c $DM/vendor/TinySoundFont/tsf.c"
CFLAGS="/nologo -fuse-ld=lld --target=i686-pc-windows-msvc -fms-compatibility /O2 -MT -D_CRT_SECURE_NO_WARNINGS"
INCLUDES="/imsvc $X/crt/include /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared"
LIBPATHS="/libpath:$X/crt/lib/x86 /libpath:$X/sdk/lib/um/x86 /libpath:$X/sdk/lib/ucrt/x86"
SDLINC="-I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision"
FLUID="-DDME_FLUIDSYNTH -DWITH_FLUIDSYNTH -I$SDL/fluidsynth-build/include -I$SDL/fluidsynth-src/include"
FLUIDLIB=$SDL/fluidsynth-build/src/libfluidsynth-3.lib
SDLLIBS="$SDL/sdl3-build/SDL3-static.lib kernel32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib \
    version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib"
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include test_miniwin.c ../../miniwin/src/ddraw.c /Fetest_miniwin.exe \
    /link /machine:x86 $LIBPATHS ddraw.lib dxguid.lib user32.lib gdi32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include $SDLINC test_paths.c ../../miniwin/src/files.c /Fetest_paths.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include $SDLINC \
    test_dsound.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c /Fetest_dsound.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS dsound.lib dxguid.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include test_acm.c ../../miniwin/src/acm.c /Fetest_acm.exe \
    /link /machine:x86 $LIBPATHS msacm32.lib winmm.lib kernel32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include $SDLINC \
    test_avifile.c ../../miniwin/src/avifile.c ../../miniwin/src/files.c /Fetest_avifile.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS vfw32.lib
clang-cl $CFLAGS -std:c11 -DDM_STATIC=1 $FLUID $INCLUDES -I../../miniwin/include $SDLINC -I$DM/include -I$DM/src \
    -I$DM/vendor/TinySoundFont test_dmusic.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c \
    ../../miniwin/src/dmusic.c ../../miniwin/src/dmengine.c ../../miniwin/src/dmreverb.c ../../miniwin/src/files.c $DMSOURCES /Fetest_dmusic.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS dxguid.lib $FLUIDLIB
clang-cl $CFLAGS -std:c11 -DDM_STATIC=1 $FLUID $INCLUDES -I../../miniwin/include $SDLINC -I$DM/include -I$DM/src \
    -I$DM/vendor/TinySoundFont capture_dmusic.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c \
    ../../miniwin/src/dmusic.c ../../miniwin/src/dmengine.c ../../miniwin/src/dmreverb.c ../../miniwin/src/files.c $DMSOURCES \
    /Fecapture_dmusic.exe /link /machine:x86 $LIBPATHS $SDLLIBS dxguid.lib $FLUIDLIB
clang-cl $CFLAGS -std:c11 -DDM_STATIC=1 $FLUID -DDM_BUILD=1 $INCLUDES -I../../miniwin/include $SDLINC -I$DM/include \
    -I$DM/src -I$DM/vendor/TinySoundFont instruments_dmusic.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c \
    ../../miniwin/src/dmusic.c ../../miniwin/src/dmengine.c ../../miniwin/src/dmreverb.c ../../miniwin/src/files.c \
    $DMSOURCES /Feinstruments_dmusic.exe /link /machine:x86 $LIBPATHS $SDLLIBS dxguid.lib $FLUIDLIB
clang-cl $CFLAGS -Wno-pointer-sign $INCLUDES -I../../port test_cinepak.c ../../port/ffmpeg/port_ffmpeg.c \
    ../../port/ffmpeg/cinepak.c ../../port/ffmpeg/indeo3.c ../../port/ffmpeg/indeo5.c ../../port/ffmpeg/ivi.c \
    ../../port/ffmpeg/ivi_dsp.c /Fetest_cinepak.exe /link /machine:x86 $LIBPATHS vfw32.lib user32.lib kernel32.lib
clang-cl $CFLAGS $INCLUDES -I../../port $SDLINC test_setup.c ../../port/port_setup.c ../../port/blast/blast.c ../../port/puff/puff.c \
    /Fetest_setup.exe /link /machine:x86 $LIBPATHS $SDLLIBS
clang-cl $CFLAGS $INCLUDES test_charset.c /Fetest_charset.exe /link /machine:x86 $LIBPATHS gdi32.lib user32.lib \
    kernel32.lib
clang-cl $CFLAGS $INCLUDES -I../../port $SDLINC test_setup_texts.c ../../port/port_setup.c ../../port/blast/blast.c \
    ../../port/puff/puff.c /Fetest_setup_texts.exe /link /machine:x86 $LIBPATHS $SDLLIBS comctl32.lib comdlg32.lib
rm -f *.obj
