#!/bin/sh

# Script to configure Qt for linking with PortaBase on the Mac
# Takes the Qt source code root directory as a parameter

cd $1
./configure -platform macx-llvm -release -opensource -no-exceptions -no-stl -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-declarative-debug -no-libtiff -no-libmng -no-openssl -nomake examples -nomake demos -no-dbus -no-dwarf2 -no-nis -arch x86_64 # -universal -sdk /Developer/SDKs/MacOSX10.4u.sdk

# When done, run the following commands to finish installation:
# make
# sudo make -j1 install

# When done, add the Qt binaries to your PATH environment variable
# For example, in ~/.bash_profile add something like:

# PATH=/usr/local/Trolltech/Qt-4.8.2/bin:$PATH
# export PATH
