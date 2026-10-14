#!/bin/sh
# Builds the FluidSynth experiment (tools/music/README.md): instruments_dmusic and capture_dmusic with FluidSynth as a
# third synthesizer, into tools/test_miniwin/fluid/ with FluidSynth's DLLs. FLUIDSYNTH_DIR is an unpacked FluidSynth
# release for 32-bit Windows (fluidsynth-v2.6.1-win10-x86-cpp11.zip from github.com/FluidSynth/fluidsynth/releases).
# Build the build-clang-x86 preset first. Then:
#   tools/test_miniwin/fluid/instruments_fluid.exe <data folder> <windows.wav> <portable.wav> <fluidsynth.wav>
#   DME_FLUIDSYNTH=1 tools/test_miniwin/fluid/capture_fluid.exe <data folder> portable <out.wav> <seconds> <segment>...
set -e
cd "$(dirname "$0")"
FS="${FLUIDSYNTH_DIR:?set FLUIDSYNTH_DIR to an unpacked FluidSynth x86 release}"
X="${XWIN_DIR:-$HOME/xwin}"
SDL=../../build-clang-x86/_deps
DM=../../port/dmusic
DMSOURCES="$DM/src/*.c $DM/src/io/*.c $DM/src/util/*.c $DM/src/thread/*.c $DM/vendor/TinySoundFont/tsf.c"
CFLAGS="/nologo -fuse-ld=lld --target=i686-pc-windows-msvc -fms-compatibility /O2 -MT -D_CRT_SECURE_NO_WARNINGS"
INCLUDES="/imsvc $X/crt/include /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared"
LIBPATHS="/libpath:$X/crt/lib/x86 /libpath:$X/sdk/lib/um/x86 /libpath:$X/sdk/lib/ucrt/x86"
SDLINC="-I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision"
SDLLIBS="$SDL/sdl3-build/SDL3-static.lib kernel32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib \
    version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib dxguid.lib $FS/lib/libfluidsynth-3.lib"
MINIWIN="../../miniwin/src/dsound.c ../../miniwin/src/window.c ../../miniwin/src/dmusic.c ../../miniwin/src/dmengine.c \
    ../../miniwin/src/dmreverb.c ../../miniwin/src/files.c"
FLAGS="-std:c11 -DDM_STATIC=1 -DDM_BUILD=1 -DWITH_FLUIDSYNTH -DDME_FLUIDSYNTH -I../../miniwin/include $SDLINC \
    -I$DM/include -I$DM/src -I$DM/vendor/TinySoundFont -I$FS/include"
mkdir -p fluid
cp "$FS/bin/libfluidsynth-3.dll" "$FS/bin/sndfile.dll" fluid/
clang-cl $CFLAGS $INCLUDES $FLAGS instruments_dmusic.c $MINIWIN $DMSOURCES /Fefluid/instruments_fluid.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS
clang-cl $CFLAGS $INCLUDES $FLAGS capture_dmusic.c $MINIWIN $DMSOURCES /Fefluid/capture_fluid.exe \
    /link /machine:x86 $LIBPATHS $SDLLIBS
rm -f *.obj fluid/*.lib fluid/*.exp
