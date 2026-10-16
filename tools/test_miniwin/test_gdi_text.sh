#!/bin/sh
# test_gdi_text.sh <Lego.TTF> <folder>: builds test_gdi_text for Windows (clang-cl and the xwin SDK: Windows' GDI)
# and for this system (miniwin's GDI, miniwin/src/posix/gdi.c; build the linux preset first, for SDL3), runs both
# in <folder> (on a Windows drive, from WSL), and compares their measurements and pictures (compare_gdi_text.py).
# Lego.TTF is the game's (from the CD): it isn't in the repository.
set -e
cd "$(dirname "$0")"
T=$(pwd)
P=$T/../..
X="${XWIN_DIR:-$HOME/xwin}"
FONT=$1
OUT=$2
mkdir -p "$OUT"
clang-cl /nologo -fuse-ld=lld --target=i686-pc-windows-msvc /O2 -MT -D_CRT_SECURE_NO_WARNINGS /imsvc $X/crt/include \
    /imsvc $X/sdk/include/ucrt /imsvc $X/sdk/include/um /imsvc $X/sdk/include/shared test_gdi_text.c \
    /Fe"$OUT/test_gdi_text.exe" /link /machine:x86 /libpath:$X/crt/lib/x86 /libpath:$X/sdk/lib/um/x86 \
    /libpath:$X/sdk/lib/ucrt/x86 gdi32.lib user32.lib kernel32.lib
rm -f *.obj
SDLB=$P/build-linux/_deps
clang -O2 -fms-extensions -Wno-microsoft-anon-tag -D_GNU_SOURCE -I$P/miniwin/include/win32 -I$P/miniwin/include \
    -I$P/miniwin/src -I$P/port -I$SDLB/sdl3-src/include -I$SDLB/sdl3-build/include-revision \
    -I$SDLB/sdl3-build/include test_gdi_text.c $P/miniwin/src/posix/*.c $P/miniwin/src/files.c \
    $P/miniwin/src/window.c $SDLB/sdl3-build/libSDL3.a -lm -lpthread -ldl -o "$OUT/test_gdi_text"
cp "$FONT" "$OUT/Lego.TTF"
cd "$OUT"
rm -f win.* lin.*
./test_gdi_text.exe Lego.TTF win
SDL_VIDEO_DRIVER=dummy ./test_gdi_text Lego.TTF lin
python3 "$T/compare_gdi_text.py" win lin
