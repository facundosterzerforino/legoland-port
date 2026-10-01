# Modern 32-bit Windows build of the port: clang-cl + lld-link, cross-compiling from Linux/WSL.
# The Windows SDK and MSVC runtime come from `xwin` (see README "Building"), by default in ~/xwin;
# set XWIN_DIR to use another location.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)
set(CMAKE_AR llvm-lib)
set(CMAKE_RC_COMPILER llvm-rc)
set(CMAKE_MT llvm-mt)

if(DEFINED ENV{XWIN_DIR})
    set(XWIN_DIR "$ENV{XWIN_DIR}")
else()
    set(XWIN_DIR "$ENV{HOME}/xwin")
endif()

set(_xwin_includes
    "/imsvc ${XWIN_DIR}/crt/include"
    "/imsvc ${XWIN_DIR}/sdk/include/ucrt"
    "/imsvc ${XWIN_DIR}/sdk/include/um"
    "/imsvc ${XWIN_DIR}/sdk/include/shared")
string(JOIN " " _xwin_includes ${_xwin_includes})

set(CMAKE_C_FLAGS_INIT "--target=i686-pc-windows-msvc -fms-compatibility ${_xwin_includes}")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "/machine:x86 /manifest:no /libpath:${XWIN_DIR}/crt/lib/x86 /libpath:${XWIN_DIR}/sdk/lib/um/x86 /libpath:${XWIN_DIR}/sdk/lib/ucrt/x86")

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
