#!/bin/sh

# Script to configure Metakit to produce a Tiger-compatible universal binary
# static library for the Mac.

export MACOSX_DEPLOYMENT_TARGET=10.4
export CXX=g++-4.0
export CXXFLAGS="-arch i386 -arch ppc"
cd metakit/builds
../unix/configure --enable-threads --disable-shared
make
sudo make install
