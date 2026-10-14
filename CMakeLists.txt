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
    # [library:filesystem] zlib's blast (zlib license, port/blast/README.md): main.z, for the first-run setup
    list(APPEND PORT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/port/blast/blast.c")
    # [library:window] [library:video] miniwin: the Windows APIs the game calls, on SDL3 (PORTING.md)
    file(GLOB MINIWIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/src/*.c")
    list(APPEND PORT_SOURCES ${MINIWIN_SOURCES})
    # Optional features with command-line switches (extensions/extensions.h)
    file(GLOB EXTENSION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/extensions/*.c")
    list(APPEND PORT_SOURCES ${EXTENSION_SOURCES})
    add_executable(legoland WIN32 ${LEGOLAND_SOURCES} ${PORT_SOURCES})
    # [library:timer] [library:filesystem] the game's clock and file calls (GetTickCount, fopen, _open,
    # CreateFileA...) go to miniwin's versions under the same names: miniwin/redirect.h comes first in every game
    # source
    set_source_files_properties(${LEGOLAND_SOURCES} PROPERTIES COMPILE_OPTIONS
        "/FI${CMAKE_CURRENT_SOURCE_DIR}/miniwin/include/miniwin/redirect.h")
    # [port] SDL3, the platform layer from phase 5 on (ROADMAP.md): a pinned release, downloaded at configure
    # time and built as a static library with the same toolchain and static C runtime. To build offline, unpack
    # the tarball somewhere and configure with -DFETCHCONTENT_SOURCE_DIR_SDL3=<folder>.
    include(FetchContent)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.tar.gz
        URL_HASH SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(SDL3)
    target_link_libraries(legoland PRIVATE SDL3::SDL3-static)
    # [library:music] GothicKit dmusic (MIT), the portable DirectMusic, with TinySoundFont (MIT) as its synthesizer
    # (port/dmusic/PORT.md). Built here rather than by its own CMakeLists, whose flags are for other compilers.
    set(DMUSIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/port/dmusic")
    file(GLOB DMUSIC_SOURCES CONFIGURE_DEPENDS "${DMUSIC_DIR}/src/*.c" "${DMUSIC_DIR}/src/io/*.c"
        "${DMUSIC_DIR}/src/util/*.c")
    add_library(dmusic STATIC ${DMUSIC_SOURCES} "${DMUSIC_DIR}/src/thread/Thread.Win32.c"
        "${DMUSIC_DIR}/src/thread/Thread.Posix.c" "${DMUSIC_DIR}/vendor/TinySoundFont/tsf.c")
    target_include_directories(dmusic PUBLIC "${DMUSIC_DIR}/include"
        PRIVATE "${DMUSIC_DIR}/src" "${DMUSIC_DIR}/vendor/TinySoundFont")
    target_compile_definitions(dmusic PUBLIC DM_STATIC=1 PRIVATE DM_BUILD=1 _CRT_SECURE_NO_WARNINGS)
    set_property(TARGET dmusic PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    set_property(TARGET dmusic PROPERTY C_STANDARD 11)
    target_link_libraries(legoland PRIVATE dmusic)
    # [library:music] FluidSynth (LGPL-2.1+, see ROADMAP.md phase 6), the portable music's synthesizer: it reads DLS
    # (gm.dls) itself by Microsoft's rules. A pinned release built into the exe without its drivers, sound file support
    # or threads; the source stays downloadable from here, as the LGPL asks. Offline: put the tarball somewhere and
    # configure with -DFETCHCONTENT_SOURCE_DIR_FLUIDSYNTH=<folder>.
    FetchContent_Declare(fluidsynth
        URL https://github.com/FluidSynth/fluidsynth/archive/refs/tags/v2.6.1.tar.gz
        URL_HASH SHA256=3d258a3bf97cc20c59eeebfe62c2432fae88adda74d3ad098681c76e0ebf446b
        EXCLUDE_FROM_ALL)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    foreach(feature alsa aufile dbus ipv6 jack ladspa libsndfile midishare network oss dsound wasapi waveout winmidi
            sdl3 pulseaudio pipewire readline threads openmp signalsmith systemd coreaudio coremidi framework
            opensles oboe libinstpatch portaudio)
        set(enable-${feature} OFF CACHE BOOL "" FORCE)
    endforeach()
    set(enable-native-dls ON CACHE BOOL "" FORCE)
    set(osal "cpp11" CACHE STRING "" FORCE)
    set(PORT_MODULE_PATH "${CMAKE_MODULE_PATH}")
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/port/cmake/fluidsynth")
    FetchContent_MakeAvailable(fluidsynth)
    set(CMAKE_MODULE_PATH "${PORT_MODULE_PATH}")
    foreach(target libfluidsynth libfluidsynth-OBJ)
        if(TARGET ${target})
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
            # its Windows version resource is for the DLL (and needs the SDK's resource headers); its -Wall is every
            # warning to clang-cl
            get_target_property(sources ${target} SOURCES)
            list(FILTER sources EXCLUDE REGEX "\.rc$")
            set_property(TARGET ${target} PROPERTY SOURCES ${sources})
            target_compile_options(${target} PRIVATE -w)
        endif()
    endforeach()
    target_link_libraries(legoland PRIVATE libfluidsynth)
    target_compile_definitions(legoland PRIVATE DME_FLUIDSYNTH)
    # miniwin's DirectMusic (miniwin/src/dmusic.c, dmengine.c) works with dmusic's internals
    target_include_directories(legoland PRIVATE "${DMUSIC_DIR}/src" "${DMUSIC_DIR}/vendor/TinySoundFont")
    # DirectInput 5 headers: the game requests version 3 at runtime (input.c) but was compiled against DX5+
    # headers (DIDEVCAPS is 0x2c bytes there; the DX3 one is 0x18 and GetCapabilities overruns it). Anything
    # below 0x0800 keeps DirectInputCreateA declared.
    # _CRT_SECURE_NO_WARNINGS: the code uses the standard C functions (fopen, strcpy, sscanf...), which are
    # portable; the Microsoft-only _s versions the CRT headers suggest aren't.
    target_compile_definitions(legoland PRIVATE LEGOLAND_PORT=1 DIRECTINPUT_VERSION=0x0500 _CRT_SECURE_NO_WARNINGS)
    target_include_directories(legoland PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/port" "${CMAKE_CURRENT_SOURCE_DIR}/extensions"
        "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/include")
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
    target_link_options(legoland PRIVATE /DEBUG /INCREMENTAL:NO)
    target_link_libraries(legoland PRIVATE
        dxguid comctl32 shell32 winmm version winspool ole32 uuid advapi32 gdi32 user32 kernel32 dbghelp)
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
