cmake_minimum_required(VERSION 3.20)
project(legoland C)

# One executable, built from every translation unit under src/legoland/.
file(GLOB LEGOLAND_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/legoland/*.c")

if(LEGOLAND_MODERN_BUILD)
    # The runnable port build (preset clang-cl-x86). Leave out the matching-only scaffolding: the dummy entry
    # point (bootstrap.c) and the stub symbols for Windows imports (imports.c); the real C runtime and Windows
    # import libraries provide those.
    list(FILTER LEGOLAND_SOURCES EXCLUDE REGEX "/src/legoland/(bootstrap|imports)\\.c$")
    # Port-only code (startup data, asm helpers, later platform layers).
    file(GLOB PORT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/port/*.c")
    add_executable(legoland WIN32 ${LEGOLAND_SOURCES} ${PORT_SOURCES})
    # DirectInput 5 headers: the game requests version 3 at runtime (input.c) but was compiled against DX5+
    # headers (DIDEVCAPS is 0x2c bytes there; the DX3 one is 0x18 and GetCapabilities overruns it). Anything
    # below 0x0800 keeps DirectInputCreateA declared.
    target_compile_definitions(legoland PRIVATE LEGOLAND_PORT=1 DIRECTINPUT_VERSION=0x0500)
    target_include_directories(legoland PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/port")
    # Static C runtime, debug info in the objects, a PDB for debuggers.
    set_property(TARGET legoland PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_options(legoland PRIVATE /Z7
        # The decompiled code mixes pointers and 32-bit integers the way MSVC6 allowed. That is fine in this
        # 32-bit build, so these stay warnings (counted, not hidden); phase 8 (64-bit) fixes them for real.
        -Wno-error=int-conversion
        -Wno-error=incompatible-function-pointer-types
        -Wno-error=return-mismatch)
    # /SAFESEH:NO: the old dinput.lib (c_dfDI* tables, DirectInputCreateA) predates SafeSEH
    target_link_options(legoland PRIVATE /DEBUG /INCREMENTAL:NO /SAFESEH:NO)
    # The modern SDK has no dinput.lib (only dinput8.lib), and the game calls DirectInputCreateA:
    # use the repo's import library for it.
    target_link_libraries(legoland PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/dinput.lib"
        ddraw dsound dxguid winmm msacm32 vfw32 version winspool ole32 uuid advapi32 gdi32 user32 kernel32)
else()
    # The MSVC6 matching build (preset msvc6): links, carries symbols for reccmp, doesn't run.
    # (The match target; runnability is a non-goal there, see cmake/msvc6-toolchain.cmake.)
    add_executable(legoland ${LEGOLAND_SOURCES})
    # movie_pool.c was compiled without optimisation in the original (frame pointers,
    # 0xCC padding between functions) but with intrinsics: memset is inlined as rep stosd.
    set_source_files_properties(src/legoland/movie_pool.c PROPERTIES COMPILE_OPTIONS "/Od;/Oi")
endif()
set_target_properties(legoland PROPERTIES SUFFIX ".exe")
