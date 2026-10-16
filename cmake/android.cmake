# Android build of the port (ROADMAP.md phase 9): arm64, Android 7.0 (API 24) and later, with the NDK's toolchain.
# ANDROID_NDK_HOME names the NDK, else the newest one under ~/android-sdk/ndk. The result is libmain.so, which
# tools/android/build_apk.sh packs into an APK with SDL's Java side.

if(DEFINED ENV{ANDROID_NDK_HOME})
    set(LEGOLAND_NDK "$ENV{ANDROID_NDK_HOME}")
else()
    file(GLOB LEGOLAND_NDKS "$ENV{HOME}/android-sdk/ndk/*")
    list(SORT LEGOLAND_NDKS COMPARE NATURAL)
    list(GET LEGOLAND_NDKS -1 LEGOLAND_NDK)
endif()
set(ANDROID_ABI arm64-v8a CACHE STRING "")
set(ANDROID_PLATFORM android-24 CACHE STRING "")
set(ANDROID_STL c++_static CACHE STRING "")
include("${LEGOLAND_NDK}/build/cmake/android.toolchain.cmake")

# Tells CMakeLists.txt this is the runnable port build, not the MSVC6 matching build.
set(LEGOLAND_MODERN_BUILD ON)
