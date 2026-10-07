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
    # [library:movie] FFmpeg's Indeo 3 and Indeo 5 decoders (LGPL-2.1+, see port/ffmpeg/README.md)
    file(GLOB PORT_FFMPEG_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/port/ffmpeg/*.c")
    list(APPEND PORT_SOURCES ${PORT_FFMPEG_SOURCES})
    # Optional features with command-line switches (extensions/extensions.h)
    file(GLOB EXTENSION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/extensions/*.c")
    list(APPEND PORT_SOURCES ${EXTENSION_SOURCES})
    add_executable(legoland WIN32 ${LEGOLAND_SOURCES} ${PORT_SOURCES})
    # DirectInput 5 headers: the game requests version 3 at runtime (input.c) but was compiled against DX5+
    # headers (DIDEVCAPS is 0x2c bytes there; the DX3 one is 0x18 and GetCapabilities overruns it). Anything
    # below 0x0800 keeps DirectInputCreateA declared.
    target_compile_definitions(legoland PRIVATE LEGOLAND_PORT=1 DIRECTINPUT_VERSION=0x0500)
    target_include_directories(legoland PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/port" "${CMAKE_CURRENT_SOURCE_DIR}/extensions")
    # Static C runtime, debug info in the objects, a PDB for debuggers.
    set_property(TARGET legoland PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_options(legoland PRIVATE /Z7
        # The decompiled code often writes "if (in bounds) p = &tbl[i]; else p = NULL;" then uses p. MSVC6 kept
        # the check and the NULL access crashed; clang treats it as impossible and deletes the check, so the
        # port silently uses tbl[i] out of bounds instead. Keep the checks: the access faults as in the
        # original and the watchdog logs it (see tools/audit_null.py for where this can happen).
        -fno-delete-null-pointer-checks
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
        ddraw dsound dxguid winmm msacm32 vfw32 version winspool ole32 uuid advapi32 gdi32 user32 kernel32 dbghelp)
    # The original exe's resources (window icon 0x65, cursor 0x7d, element dialog 0x75, version), copied from
    # external/legoland.exe at build time rather than committed (tools/extract_resources.py).
    set(LEGOLAND_ORIGINAL_EXE "${CMAKE_CURRENT_SOURCE_DIR}/external/legoland.exe")
    if(EXISTS "${LEGOLAND_ORIGINAL_EXE}")
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(LEGOLAND_RES "${CMAKE_CURRENT_BINARY_DIR}/legoland_original.res")
        add_custom_command(OUTPUT "${LEGOLAND_RES}"
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_resources.py"
                    "${LEGOLAND_ORIGINAL_EXE}" "${LEGOLAND_RES}"
            DEPENDS "${LEGOLAND_ORIGINAL_EXE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_resources.py"
            COMMENT "Extracting the original exe's resources")
        add_custom_target(legoland_resources DEPENDS "${LEGOLAND_RES}")
        add_dependencies(legoland legoland_resources)
        target_link_options(legoland PRIVATE "${LEGOLAND_RES}")
        set_property(TARGET legoland APPEND PROPERTY LINK_DEPENDS "${LEGOLAND_RES}")
    else()
        message(WARNING "external/legoland.exe not found: the port exe gets no icon, cursor or dialog resources")
    endif()
else()
    # The MSVC6 matching build (preset msvc6): links, carries symbols for reccmp, doesn't run.
    # (The match target; runnability is a non-goal there, see cmake/msvc6-toolchain.cmake.)
    add_executable(legoland ${LEGOLAND_SOURCES})
    # movie_pool.c was compiled without optimisation in the original (frame pointers,
    # 0xCC padding between functions) but with intrinsics: memset is inlined as rep stosd.
    set_source_files_properties(src/legoland/movie_pool.c PROPERTIES COMPILE_OPTIONS "/Od;/Oi")
endif()
set_target_properties(legoland PROPERTIES SUFFIX ".exe")
