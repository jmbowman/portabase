#!/bin/sh

# Configure and compile Metakit for Android ARM devices

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    export TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
elif [[ "$OSTYPE" == "darwin"* ]]; then
    export TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/darwin-x86_64
fi

# Only choose one of these, depending on your device...
#export TARGET=aarch64-linux-android
export TARGET=armv7a-linux-androideabi
#export TARGET=i686-linux-android
#export TARGET=x86_64-linux-android

# If selecting armv7a, this is needed for most of the binaries
export BASE_TARGET=arm-linux-androideabi
# Otherwise, use this
# export BASE_TARGET=$TARGET

# This is needed for armv7a
export CXXFLAGS="-fPIC"

# Set this to your minSdkVersion.
export API=21

# Configure and build.
export AR=$TOOLCHAIN/bin/$BASE_TARGET-ar
export AS=$TOOLCHAIN/bin/$BASE_TARGET-as
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/$BASE_TARGET-ld
export RANLIB=$TOOLCHAIN/bin/$BASE_TARGET-ranlib
export STRIP=$TOOLCHAIN/bin/$BASE_TARGET-strip

cd metakit/builds
../unix/configure --disable-shared --enable-threads --host $TARGET
make
#abuild -f ../../packaging/android/libraries.json -R
cd ../..
