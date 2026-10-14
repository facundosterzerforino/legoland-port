# [port] Stands in for CMake's module while FluidSynth configures (CMakeLists.txt): FluidSynth includes it for its
# own installer packages, which the port does not build, and the real one queries the Windows registry for the
# MSVC runtime, which fails when cross-compiling.
