#!/bin/bash

# Configure and compile Metakit for Android ARM devices

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    export TOOLCHAIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64
elif [[ "$OSTYPE" == "darwin"* ]]; then
    export TOOLCHAIN=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64
fi

# Set this to your minSdkVersion.
export API=21

for TARGET in armeabi-v7a arm64-v8a x86 x86_64
do
    if [[ "$TARGET" == "armeabi-v7a" ]]; then
        export TARGET=armv7a-linux-androideabi
        export BASE_TARGET=arm-linux-androideabi
        export CXXFLAGS="-fPIC"
    elif [[ "$TARGET" == "arm64-v8a" ]]; then
        export TARGET=aarch64-linux-android
        export BASE_TARGET=$TARGET
    elif [[ "$TARGET" == "x86" ]]; then
        export TARGET=i686-linux-android
        export BASE_TARGET=$TARGET
    elif [[ "$TARGET" == "x86_64" ]]; then
        export TARGET=x86_64-linux-android
        export BASE_TARGET=$TARGET
    fi

    # Configure and build.
    export AR=$TOOLCHAIN/bin/$BASE_TARGET-ar
    export AS=$TOOLCHAIN/bin/$BASE_TARGET-as
    export CC=$TOOLCHAIN/bin/$TARGET$API-clang
    export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
    export LD=$TOOLCHAIN/bin/$BASE_TARGET-ld
    export RANLIB=$TOOLCHAIN/bin/$BASE_TARGET-ranlib
    export STRIP=$TOOLCHAIN/bin/$BASE_TARGET-strip

    cd metakit/builds

    # Remove intermediate artifacts from prior build
    rm *.o demo dump myio

    ../unix/configure --enable-threads --host $TARGET
    make
    #abuild -f ../../packaging/android/libraries.json -R
    mv libmk4.so libmk4_$TARGET.so
    cd ../..
done
