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
    # [library:filesystem] zlib's puff (zlib license, port/puff/README.md): the Czech CD's lego.pak (Deflate)
    list(APPEND PORT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/port/puff/puff.c")
    # [library:window] [library:video] miniwin: the Windows APIs the game calls, on SDL3 (PORTING.md)
    file(GLOB MINIWIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/src/*.c")
    list(APPEND PORT_SOURCES ${MINIWIN_SOURCES})
    # Optional features with command-line switches (extensions/extensions.h)
    file(GLOB EXTENSION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/extensions/*.c")
    list(APPEND PORT_SOURCES ${EXTENSION_SOURCES})
    if(NOT WIN32)
        # [port] Windows-only: the setup window and the watchdog (its thread sampling and debug registers); their
        # Linux counterparts come later (ROADMAP.md phase 8, step 4)
        list(FILTER PORT_SOURCES EXCLUDE REGEX "/port/port_(setup_win|watchdog)\\.c$")
        # [port] the Win32 functions on POSIX (miniwin/src/posix/posix.h), the entry point and the stand-ins for
        # those two (port/port_posix.c)
        file(GLOB MINIWIN_POSIX_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/src/posix/*.c")
        list(APPEND PORT_SOURCES ${MINIWIN_POSIX_SOURCES})
        set_source_files_properties(${MINIWIN_POSIX_SOURCES} PROPERTIES COMPILE_DEFINITIONS _GNU_SOURCE)
    else()
        list(FILTER PORT_SOURCES EXCLUDE REGEX "/port/port_(posix|setup_sdl)\\.c$")
    endif()
    if(ANDROID)
        # [port] Android: the game is the library SDL's Java side loads (libmain.so) and calls SDL_main in
        add_library(legoland SHARED ${LEGOLAND_SOURCES} ${PORT_SOURCES})
        set_target_properties(legoland PROPERTIES OUTPUT_NAME main)
    else()
        add_executable(legoland WIN32 ${LEGOLAND_SOURCES} ${PORT_SOURCES})
    endif()
    # [library:timer] [library:filesystem] the game's clock and file calls (GetTickCount, fopen, _open,
    # CreateFileA...) go to miniwin's versions under the same names: miniwin/redirect.h comes first in every game
    # source
    if(WIN32)
        set_source_files_properties(${LEGOLAND_SOURCES} PROPERTIES COMPILE_OPTIONS
            "/FI${CMAKE_CURRENT_SOURCE_DIR}/miniwin/include/miniwin/redirect.h")
    else()
        set_source_files_properties(${LEGOLAND_SOURCES} PROPERTIES COMPILE_OPTIONS
            "-include;${CMAKE_CURRENT_SOURCE_DIR}/miniwin/include/miniwin/redirect.h")
    endif()
    # [port] SDL3, the platform layer from phase 5 on (ROADMAP.md): a pinned release, downloaded at configure
    # time and built as a static library with the same toolchain and static C runtime. To build offline, unpack
    # the tarball somewhere and configure with -DFETCHCONTENT_SOURCE_DIR_SDL3=<folder>.
    include(FetchContent)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    if(EMSCRIPTEN)
        # one memory for all the threads: every library is compiled for it
        add_compile_options(-pthread)
        set(SDL_PTHREADS ON CACHE BOOL "" FORCE)
    endif()
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
    if(ANDROID)
        # all of it: its JNI functions are called from Java only, so nothing in C would pull them in
        target_link_libraries(legoland PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,SDL3::SDL3-static>")
    else()
        target_link_libraries(legoland PRIVATE SDL3::SDL3-static)
    endif()
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
    if(NOT WIN32)
        # [port] off Windows: <windows.h>, <ddraw.h>... are miniwin's, the SDK declarations the code uses (generated by
        # tools/linux/gen_win32.py); the code's Microsoft C (anonymous struct members, __int64...) as clang-cl takes it
        target_include_directories(legoland BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/miniwin/include/win32")
        # and its chars signed, as MSVC's (ARM's are unsigned), its wchar_t 16 bits, as Windows' (L"..." are the WCHAR
        # strings DirectMusic's names are), and no type-based alias analysis, as MSVC's and clang-cl's (the decompiled code
        # reads memory through other types all the time; clang elsewhere would assume it doesn't, at -O2)
        target_compile_options(legoland PRIVATE -fms-extensions -Wno-microsoft-anon-tag -fsigned-char -fshort-wchar
            -fno-strict-aliasing)
        # with a 16-bit wchar_t, clang would turn the code's loops over 16-bit strings into calls to the C library's
        # wcslen (and its kin), which are 32-bit here: they're not builtins, then
        target_compile_options(legoland PRIVATE -fno-builtin-wcslen -fno-builtin-wcscpy -fno-builtin-wcsncpy -fno-builtin-wcscat -fno-builtin-wcscmp -fno-builtin-wcschr -fno-builtin-wcsrchr -fno-builtin-wmemcpy -fno-builtin-wmemmove -fno-builtin-wmemset -fno-builtin-wmemcmp -fno-builtin-wmemchr)
        if(EMSCRIPTEN)
            target_compile_options(legoland PRIVATE -pthread)
        endif()
    endif()
    # Static C runtime, debug info in the objects, a PDB for debuggers (Windows); debug info (Linux).
    set_property(TARGET legoland PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    if(WIN32)
        target_compile_options(legoland PRIVATE /Z7)
    else()
        target_compile_options(legoland PRIVATE -g)
    endif()
    target_compile_options(legoland PRIVATE
        # The decompiled code often writes "if (in bounds) p = &tbl[i]; else p = NULL;" then uses p. MSVC6 kept
        # the check and the NULL access crashed; clang treats it as impossible and deletes the check, so the
        # port silently uses tbl[i] out of bounds instead. Keep the checks: the access faults as in the
        # original and the watchdog logs it (see tools/audit_null.py for where this can happen).
        -fno-delete-null-pointer-checks
        # The decompiled code mixes pointers and 32-bit integers the way MSVC6 allowed. That is fine in this
        # 32-bit build, so these stay warnings (counted, not hidden); phase 8 (64-bit) fixes them for real.
        -Wno-error=int-conversion
        -Wno-error=incompatible-function-pointer-types
        -Wno-error=return-mismatch
        # and as MSVC6 took them: a missing return value, a format that isn't a literal (errors in the NDK's flags)
        -Wno-error=return-type
        -Wno-error=format-security)
    if(WIN32)
        target_link_options(legoland PRIVATE /DEBUG /INCREMENTAL:NO)
        target_link_libraries(legoland PRIVATE
            dxguid comctl32 comdlg32 shell32 winmm version winspool ole32 uuid advapi32 gdi32 user32 kernel32 dbghelp)
    else()
        if(ANDROID)
            # FluidSynth is C++: linked as C++, with the NDK's C++ library
            target_link_libraries(legoland PRIVATE m dl log android)
            set_target_properties(legoland PROPERTIES LINKER_LANGUAGE CXX)
        elseif(EMSCRIPTEN)
            # [port] WebAssembly: threads (the game's are real ones: the music thread, timers), a stack the size of
            # Windows' (the game's locals were sized for it), the heap growing as needed; FluidSynth is C++
            set_target_properties(legoland PROPERTIES LINKER_LANGUAGE CXX)
            target_link_options(legoland PRIVATE -pthread -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=268435456
                -sSTACK_SIZE=4194304 -sDEFAULT_PTHREAD_STACK_SIZE=1048576 -sPTHREAD_POOL_SIZE=8 -sEXIT_RUNTIME=1)
            # the decompiled code calls functions through pointers of other types: a value returned where the
            # pointer says void (the sound buffer's Stop, the objectives' event handlers), fewer or more arguments
            # (callbacks kept in void * tables, unprototyped calls). x86 doesn't mind; WebAssembly checks every call
            # through a pointer and stops the program ("function signature mismatch"). Emulated, such a call works
            # as on x86 (the values converted as through memory). The list: build with -Wcast-function-type.
            target_link_options(legoland PRIVATE -sEMULATE_FUNCTION_POINTER_CASTS=1)
            if(LEGOLAND_WEB STREQUAL "node")
                # Node.js: the real file system, from the current folder (-port-selftest in an installed game's)
                target_link_options(legoland PRIVATE -sNODERAWFS=1 -sENVIRONMENT=node
                    "--pre-js=${CMAKE_CURRENT_SOURCE_DIR}/port/web/node-pre.js")
                set_property(TARGET legoland APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/port/web/node-pre.js")
                target_compile_definitions(legoland PRIVATE LEGOLAND_WEB_NODE=1)
            else()
                # a browser: the game runs on a worker's thread (PROXY_TO_PTHREAD: main is started on one), the page's
                # own thread staying free for the page: the drawing (WebGL, its calls made there: OFFSCREEN_FRAMEBUFFER),
                # the input, the sound, and the storage, which WebKit (Safari, iPhones, iPads) does for every thread
                # there. The game's files are in the origin's private storage (WASMFS on OPFS); the page is
                # port/web/shell.html. iOS's browsers don't start a page whose memory may grow past 1 GB (SDL's
                # docs/README-emscripten.md).
                target_link_options(legoland PRIVATE -sPROXY_TO_PTHREAD=1 -sOFFSCREEN_FRAMEBUFFER=1
                    -sMAXIMUM_MEMORY=1073741824 -sWASMFS=1
                    -sFORCE_FILESYSTEM=1 -sENVIRONMENT=web,worker "-sEXPORTED_RUNTIME_METHODS=['callMain','FS']"
                    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/port/web/shell.html"
                    "--post-js=${CMAKE_CURRENT_SOURCE_DIR}/port/web/offscreen_resize.js")
                set_property(TARGET legoland APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/port/web/shell.html"
                    "${CMAKE_CURRENT_SOURCE_DIR}/port/web/offscreen_resize.js")
                # the page's helper, next to it (port/web/README.md)
                add_custom_command(TARGET legoland POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
                    "${CMAKE_CURRENT_SOURCE_DIR}/port/web/coi-serviceworker.js"
                    "${CMAKE_CURRENT_SOURCE_DIR}/port/web/manifest.webmanifest" "${CMAKE_CURRENT_BINARY_DIR}/")
                target_compile_definitions(legoland PRIVATE LEGOLAND_WEB_BROWSER=1)
                set_target_properties(legoland PROPERTIES SUFFIX ".html")
            endif()
        else()
            if(LEGOLAND_LINUX_PORTABLE)
                # Zig's C++ library (libc++), linked in statically
                set_target_properties(legoland PROPERTIES LINKER_LANGUAGE CXX)
                target_link_libraries(legoland PRIVATE m pthread dl)
            else()
                target_link_libraries(legoland PRIVATE m pthread dl stdc++) # stdc++: FluidSynth is C++
            endif()
        endif()
    endif()
    # The original exe's resources (window icon 0x65, cursor 0x7d, element dialog 0x75, version), copied from
    # external/legoland.exe at build time rather than committed (tools/extract_resources.py).
    set(LEGOLAND_ORIGINAL_EXE "${CMAKE_CURRENT_SOURCE_DIR}/external/legoland.exe")
    if(NOT WIN32)
        # [port] Linux, Android, the web: no PE resources (the APK's icon: tools/android/build_apk.sh)
    elseif(EXISTS "${LEGOLAND_ORIGINAL_EXE}")
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
if(ANDROID OR EMSCRIPTEN OR LEGOLAND_LINUX_PORTABLE)
    enable_language(CXX)
elseif(WIN32 OR NOT LEGOLAND_MODERN_BUILD)
    set_target_properties(legoland PROPERTIES SUFFIX ".exe")
endif()
