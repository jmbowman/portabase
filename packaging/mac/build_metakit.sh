#!/bin/sh

# Script to configure Metakit as a static library.  By default, only targets
# the build system's native processor architecture.

# To produce a Leopard-compatible universal binary library instead, pass the
# string "universal" as a parameter.  You'll need to either be running a
# version of Xcode earlier than Xcode 4, or go through some interesting hoops
# to reinstate support for older compilers and target SDKs.

set -ex

if [ "$1" == "universal" ]; then
    export MACOSX_DEPLOYMENT_TARGET=10.5
    export CXX=g++-4.0
    export CXXFLAGS="-arch i386 -arch ppc"
fi
cd metakit/builds
../unix/configure --enable-threads --disable-shared
make clean
make
sudo make install
