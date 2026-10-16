# The Linux release build (preset linux-portable): as cmake/linux.cmake, but with Zig's clang linking against glibc
# 2.31, so that the binary runs on Linux distributions from 2020 on, not only on the one it was built on
# (tools/linux/zigcc; Zig: https://ziglang.org, ZIG names it, else ~/zig/zig). The C++ library goes in statically.

set(CMAKE_C_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../tools/linux/zigcc")
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_LIST_DIR}/../tools/linux/zigcxx")
set(LEGOLAND_LINUX_PORTABLE ON)
# the system's libraries where Debian and Ubuntu keep them, for SDL's checks (it loads X11's, ALSA's... at run time)
set(CMAKE_LIBRARY_ARCHITECTURE x86_64-linux-gnu)
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH /usr/lib/x86_64-linux-gnu)
# Zig's linker doesn't write the dependency files CMake asks of it
set(CMAKE_LINK_DEPENDS_USE_LINKER OFF)

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)
