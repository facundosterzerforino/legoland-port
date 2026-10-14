# 64-bit Windows build of the port (ROADMAP.md phase 8): clang-cl + lld-link, cross-compiling from Linux/WSL,
# like clang-cl-x86.cmake. `xwin --arch x86,x86_64 splat` gets both architectures' SDK and runtime.
# The Windows SDK and MSVC runtime come from `xwin` (see README "Building"), by default in ~/xwin;
# set XWIN_DIR to use another location.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
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
    "/imsvc ${XWIN_DIR}/sdk/include/shared"
    "/imsvc ${XWIN_DIR}/sdk/include/winrt")
string(JOIN " " _xwin_includes ${_xwin_includes})

set(CMAKE_C_FLAGS_INIT "--target=x86_64-pc-windows-msvc -fms-compatibility ${_xwin_includes}")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}") # SDL3 (3rdparty) has a few C++ files
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "/machine:x64 /manifest:no /libpath:${XWIN_DIR}/crt/lib/x86_64 /libpath:${XWIN_DIR}/sdk/lib/um/x86_64 /libpath:${XWIN_DIR}/sdk/lib/ucrt/x86_64")

# CMake's compiler checks build in Debug by default, which links the debug C runtime (msvcrtd.lib); xwin leaves the
# debug runtime out.
set(CMAKE_TRY_COMPILE_CONFIGURATION Release)

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
