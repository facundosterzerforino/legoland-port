#!/bin/sh
# Builds the miniwin tests that run in a browser, with Emscripten (. ~/emsdk/emsdk_env.sh first) and the SDL3 the web
# preset fetched (build build-web first), linked as the game's browser build is (CMakeLists.txt: the game's thread a
# worker's, its WebGL calls made on the page's thread): test_present (the picture on the screen). Into
# build-web/tests/; tools/web/presenttest.py runs them.
set -e
cd "$(dirname "$0")"
P=$(pwd)/../..
SDLB=$P/build-web/_deps
OUT=$P/build-web/tests
mkdir -p "$OUT"
WIDE="-fno-builtin-wcslen -fno-builtin-wcscpy -fno-builtin-wcsncpy -fno-builtin-wcscat -fno-builtin-wcscmp -fno-builtin-wcschr -fno-builtin-wcsrchr -fno-builtin-wmemcpy -fno-builtin-wmemmove -fno-builtin-wmemset -fno-builtin-wmemcmp -fno-builtin-wmemchr"
CFLAGS="-O2 -pthread -fno-strict-aliasing $WIDE -fms-extensions -Wno-microsoft-anon-tag -D_GNU_SOURCE -DLEGOLAND_WEB_BROWSER=1 \
    -I$P/miniwin/include/win32 -I$P/miniwin/include -I$P/miniwin/src -I$P/port -I$SDLB/sdl3-src/include \
    -I$SDLB/sdl3-build/include-revision -I$SDLB/sdl3-build/include"
LINK="-pthread -sPROXY_TO_PTHREAD=1 -sOFFSCREEN_FRAMEBUFFER=1 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=1073741824 \
    -sPTHREAD_POOL_SIZE=4 -sEXIT_RUNTIME=1 -sWASMFS=1 -sFORCE_FILESYSTEM=1 -sENVIRONMENT=web,worker \
    "-sEXPORTED_RUNTIME_METHODS=['FS']" --shell-file=present_shell.html --post-js=$P/port/web/offscreen_resize.js"
POSIX="$P/miniwin/src/posix/*.c $P/miniwin/src/files.c $P/miniwin/src/window.c"
emcc $CFLAGS test_present.c $POSIX $SDLB/sdl3-build/libSDL3.a $LINK -o "$OUT/test_present.html"
echo "built in $OUT: test_present"
