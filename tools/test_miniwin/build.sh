#!/bin/sh
# Builds test_miniwin.exe, test_paths.exe, test_dsound.exe, test_acm.exe and test_avifile.exe (clang-cl + lld-link with the xwin SDK, like the clang-cl-x86 preset).
# test_paths links the SDL3 that the port build fetched: build build-clang-x86 first.
set -e
cd "$(dirname "$0")"
X="${XWIN_DIR:-$HOME/xwin}"
SDL=../../build-clang-x86/_deps
CFLAGS="/nologo -fuse-ld=lld --target=i686-pc-windows-msvc -fms-compatibility /O2 -MT -D_CRT_SECURE_NO_WARNINGS"
INCLUDES="/imsvc $X/crt/include /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared"
LIBPATHS="/libpath:$X/crt/lib/x86 /libpath:$X/sdk/lib/um/x86 /libpath:$X/sdk/lib/ucrt/x86"
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include test_miniwin.c ../../miniwin/src/ddraw.c /Fetest_miniwin.exe \
    /link /machine:x86 $LIBPATHS ddraw.lib dxguid.lib user32.lib gdi32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include -I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision \
    test_paths.c ../../miniwin/src/files.c /Fetest_paths.exe \
    /link /machine:x86 $LIBPATHS $SDL/sdl3-build/SDL3-static.lib kernel32.lib user32.lib gdi32.lib winmm.lib \
    imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include -I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision \
    test_dsound.c ../../miniwin/src/dsound.c ../../miniwin/src/window.c /Fetest_dsound.exe \
    /link /machine:x86 $LIBPATHS $SDL/sdl3-build/SDL3-static.lib dsound.lib dxguid.lib kernel32.lib user32.lib \
    gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include test_acm.c ../../miniwin/src/acm.c /Fetest_acm.exe     /link /machine:x86 $LIBPATHS msacm32.lib winmm.lib kernel32.lib
clang-cl $CFLAGS $INCLUDES -I../../miniwin/include -I$SDL/sdl3-src/include -I$SDL/sdl3-build/include-revision     test_avifile.c ../../miniwin/src/avifile.c ../../miniwin/src/files.c /Fetest_avifile.exe     /link /machine:x86 $LIBPATHS $SDL/sdl3-build/SDL3-static.lib vfw32.lib kernel32.lib user32.lib gdi32.lib     winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib
rm -f *.obj
