#!/bin/sh
# tools/android/build_apk.sh: builds the Android app (ROADMAP.md phase 9), build-android/LEGOLAND-port.apk, without
# Gradle: the game's libmain.so for each of ABIS (arm64-v8a for phones and tablets, x86_64 for emulators and
# Chromebooks: build-android, build-android-x86_64), SDL's Java side with the app's activity (javac, d8), the
# manifest and resources (aapt2), signed (apksigner) with a key of this machine's (~/.android/legoland-port.keystore,
# made on the first run; an update must be signed with the same key). Needs the Android SDK in ~/android-sdk (or
# ANDROID_HOME): build-tools 35.0.0, platform 35, an NDK; and a JDK (17).
set -e
cd "$(dirname "$0")/../.."
P=$(pwd)
SDK=${ANDROID_HOME:-$HOME/android-sdk}
BT=$SDK/build-tools/35.0.0
JAR=$SDK/platforms/android-35/android.jar
NDK=${ANDROID_NDK_HOME:-$(ls -d $SDK/ndk/* | sort -V | tail -1)}
STRIP=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip
B=$P/build-android
OUT=$B/apk
KEY=$HOME/.android/legoland-port.keystore

ABIS=${ABIS:-arm64-v8a x86_64}
rm -rf $OUT && mkdir -p $OUT/classes $OUT/dex $OUT/res/values $OUT/res/mipmap-xxxhdpi $OUT/compiled $OUT/apk
for ABI in $ABIS; do
    if [ $ABI = arm64-v8a ]; then
        D=$B
        [ -f $D/build.ninja ] || cmake --preset android
    else
        D=$B-$ABI
        [ -f $D/build.ninja ] || cmake -S $P -B $D -G Ninja --toolchain $P/cmake/android.cmake -DANDROID_ABI=$ABI \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo
    fi
    cmake --build $D --target legoland
    mkdir -p $OUT/apk/lib/$ABI
    $STRIP --strip-debug -o $OUT/apk/lib/$ABI/libmain.so $D/libmain.so
done

# Java: SDL's (from the SDL3 release the build fetched) and the activity
find $B/_deps/sdl3-src/android-project/app/src/main/java -name '*.java' > $OUT/sources.txt
echo $P/port/android/java/LegolandActivity.java >> $OUT/sources.txt
javac -nowarn -source 8 -target 8 -bootclasspath $JAR -classpath $JAR -d $OUT/classes @$OUT/sources.txt 2>&1 \
    | grep -v "^warning: \[options\]" | grep -v "^Note:" | grep -v "^[0-9] warnings\?$" || true
$BT/d8 --release --min-api 24 --lib $JAR --output $OUT/dex $(find $OUT/classes -name '*.class')

# resources: the name, and the original exe's icon (not committed: made from external/legoland.exe)
cat > $OUT/res/values/strings.xml <<'EOF'
<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">LEGOLAND</string>
</resources>
EOF
if [ -f $P/external/legoland.exe ]; then
    python3 $P/tools/android/exe_icon.py $P/external/legoland.exe $OUT/res/mipmap-xxxhdpi/ic_launcher.png 192
else
    echo "external/legoland.exe not found: the app gets Android's default icon"
fi
$BT/aapt2 compile --dir $OUT/res -o $OUT/compiled/res.zip
VERSION=$(git -C $P rev-parse --short HEAD)
CODE=$(git -C $P rev-list --count HEAD)
cp $P/port/android/AndroidManifest.xml $OUT/manifest.xml
[ -f $P/external/legoland.exe ] || sed -i 's/ android:icon="@mipmap\/ic_launcher"//' $OUT/manifest.xml
# the music's instruments where there's no gm.dls: GeneralUser GS (tools/music/fetch_soundfont.py), an asset
mkdir -p $OUT/assets
cp "$(python3 $P/tools/music/fetch_soundfont.py)" $OUT/assets/
$BT/aapt2 link -I $JAR --manifest $OUT/manifest.xml --min-sdk-version 24 --target-sdk-version 35 \
    --version-code $CODE --version-name "port $VERSION" -A $OUT/assets -o $OUT/unsigned.apk $OUT/compiled/res.zip

# the code: the dex and the game's libraries (without their debug information, above)
cp $OUT/dex/classes.dex $OUT/apk/
(cd $OUT/apk && zip -q -r ../unsigned.apk classes.dex lib)
$BT/zipalign -P 16 -f 4 $OUT/unsigned.apk $OUT/aligned.apk

if [ ! -f $KEY ]; then
    mkdir -p $(dirname $KEY)
    keytool -genkeypair -keystore $KEY -storepass legoland -keypass legoland -alias legoland -keyalg RSA \
        -keysize 2048 -validity 10000 -dname "CN=LEGOLAND port" > /dev/null 2>&1
fi
$BT/apksigner sign --ks $KEY --ks-pass pass:legoland --key-pass pass:legoland --out $B/LEGOLAND-port.apk $OUT/aligned.apk
$BT/apksigner verify $B/LEGOLAND-port.apk
echo "$B/LEGOLAND-port.apk: $(du -h $B/LEGOLAND-port.apk | cut -f1), version code $CODE (port $VERSION)"
