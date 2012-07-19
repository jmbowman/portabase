#!/bin/sh

# Script to configure Metakit as a static library.  Only targets 64-bit Intel
# processors, which is what Apple currently recommends and is what Xcode 4 is
# set up to accomplish.

# To produce a Tiger-compatible universal binary library instead, you'll need
# to use the commented out settings instead and either be running a version of
# Xcode earlier than Xcode 4, or go through some interesting hoops to
# reinstate support for older compilers and target SDKs.

#export MACOSX_DEPLOYMENT_TARGET=10.4
#export CXX=g++-4.0
#export CXXFLAGS="-arch i386 -arch ppc"
export CXXFLAGS="-arch x86_64"
cd metakit/builds
../unix/configure --enable-threads --disable-shared
make
sudo make install
