# WebAssembly build of the port (ROADMAP.md phase 10), with Emscripten (EMSDK names its SDK, else ~/emsdk). For a
# browser (LEGOLAND_WEB=browser, preset web): the page port/web/shell.html, the game's files in the browser's private
# storage (OPFS); or for Node.js (LEGOLAND_WEB=node, preset web-node): the real file system, for -port-selftest.

if(DEFINED ENV{EMSDK})
    set(LEGOLAND_EMSDK "$ENV{EMSDK}")
else()
    set(LEGOLAND_EMSDK "$ENV{HOME}/emsdk")
endif()
include("${LEGOLAND_EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)
