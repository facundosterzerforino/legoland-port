#!/bin/sh
# tools/android/selftest.sh <game folder> <report folder>: runs the app's -port-selftest on an Android emulator (the
# AVD "legotest", x86_64, made with avdmanager), with the installed game copied into the app's folder, and pulls the
# reports (port-selftest*.txt) into <report folder>. Builds the APK first (build_apk.sh). Needs KVM. The game folder
# is the game's data: it's only copied onto the emulator, and wiped from it at the end.
set -e
cd "$(dirname "$0")/../.."
P=$(pwd)
SDK=${ANDROID_HOME:-$HOME/android-sdk}
ADB=$SDK/platform-tools/adb
PKG=io.github.facundosterzerforino.legolandport
GAME=$1
OUT=$2
APPDIR=/sdcard/Android/data/$PKG/files

sh $P/tools/android/build_apk.sh > /tmp/legoland-apk.log 2>&1 || { tail -20 /tmp/legoland-apk.log; exit 1; }
tail -1 /tmp/legoland-apk.log
$SDK/emulator/emulator -avd legotest -no-window -no-audio -no-snapshot -no-boot-anim -gpu swiftshader_indirect \
    > /tmp/legoland-emulator.log 2>&1 &
EMU=$!
trap 'kill $EMU 2>/dev/null || true' EXIT
$ADB wait-for-device
until [ "$($ADB shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')" = "1" ]; do sleep 2; done
echo "emulator: Android $($ADB shell getprop ro.build.version.release | tr -d '\r'), $($ADB shell getprop ro.product.cpu.abi | tr -d '\r')"
$ADB uninstall $PKG > /dev/null 2>&1 || true # a clean start: no folder left from before
$ADB install -r $P/build-android/LEGOLAND-port.apk > /dev/null
# the selftest ends the app: wait for its process to go
run() {
    $ADB shell am start -W -n $PKG/.LegolandActivity --es args -port-selftest > /dev/null
    sleep 3
    i=0
    while [ -n "$($ADB shell pidof $PKG 2>/dev/null | tr -d '\r')" ] && [ $i -lt 300 ]; do sleep 1; i=$((i + 1)); done
}
# the app makes its folder itself (one adb made would be the shell's, and the app couldn't read what's in it): a
# first selftest, without the game's files, ends at once
run
$ADB push "$GAME"/. $APPDIR/ > /dev/null
$ADB shell rm -f "$APPDIR/port-selftest*.txt"
run
mkdir -p "$OUT"
for f in port-selftest.txt port-selftest-odf.txt port-selftest-bnv.txt port-selftest-loc.txt port-selftest-lms.txt \
    port-selftest-mesh.txt legoland-port-trace.txt; do
    $ADB pull $APPDIR/$f "$OUT/$f" > /dev/null 2>&1 || echo "no $f"
done
$ADB logcat -d | grep -iE "SDL|legoland|crash|FATAL" | tail -20 > "$OUT/logcat.txt" || true
$ADB shell rm -rf $APPDIR
$ADB uninstall $PKG > /dev/null
$ADB emu kill > /dev/null 2>&1 || true
wait $EMU 2>/dev/null || true
trap - EXIT
tail -1 "$OUT/port-selftest.txt" 2>/dev/null || echo "no report"
