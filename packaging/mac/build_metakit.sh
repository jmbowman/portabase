#!/bin/sh

# Script to configure Metakit as a static library.  Only targets 64-bit Intel
# processors, which is what Apple currently recommends and is what Xcode 4 is
# set up to accomplish.

# To produce a Leopard-compatible universal binary library instead, pass the
# string "universal" as a parameter.  You'll need to either be running a
# version of Xcode earlier than Xcode 4, or go through some interesting hoops
# to reinstate support for older compilers and target SDKs.

if [ "$1" == "universal" ]; then
    export MACOSX_DEPLOYMENT_TARGET=10.5
    export CXX=g++-4.0
    export CXXFLAGS="-arch i386 -arch ppc"
else
    export CXXFLAGS="-arch x86_64"
fi
cd metakit/builds
../unix/configure --enable-threads --disable-shared
make
sudo make install
