# Linux build of the port (ROADMAP.md phase 8, step 4): the host's clang, 64-bit. SDL3 picks its video and audio
# backends from the development packages it finds (X11/Wayland, ALSA/PulseAudio/PipeWire); without them it builds
# with only its dummy drivers, enough for -port-selftest.

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++) # SDL3 and FluidSynth have C++ files

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)
