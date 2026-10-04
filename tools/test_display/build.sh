#!/bin/sh
# Builds test_display.exe (clang-cl + lld-link with the xwin SDK, like the clang-cl-x86 preset).
set -e
cd "$(dirname "$0")"
X="${XWIN_DIR:-$HOME/xwin}"
clang-cl /nologo -fuse-ld=lld --target=i686-pc-windows-msvc -fms-compatibility /O2 -D_CRT_SECURE_NO_WARNINGS \
    /imsvc "$X/crt/include" /imsvc "$X/sdk/include/ucrt" /imsvc "$X/sdk/include/um" /imsvc "$X/sdk/include/shared" \
    -I../../port test_display.c ../../port/port_display.c ../../port/port_trace.c /Fetest_display.exe \
    /link /machine:x86 /libpath:"$X/crt/lib/x86" /libpath:"$X/sdk/lib/um/x86" /libpath:"$X/sdk/lib/ucrt/x86" \
    ddraw.lib dxguid.lib user32.lib gdi32.lib
rm -f *.obj
