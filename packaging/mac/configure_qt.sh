#!/bin/sh

# Script to configure Qt for linking with PortaBase on the Mac
# Takes the Qt source code root directory as a parameter

# Compiles for 64-bit Intel only by default.  To compile a universal binary
# version that works on Leopard and later, pass the string "universal" as the
# second paramater; you'll need to have a suitably old or hacked version of
# Xcode.

# Qt 4.8.0 and higher no longer successfully compiles with support for Tiger;
# for examples, see https://bugreports.qt-project.org/browse/QTBUG-18947 and
# https://bugreports.qt-project.org/browse/QTBUG-25417.  To support Tiger,
# you'd have to run an earlier version of Qt, uncomment the end of the first
# configure command below, and change that line's -sdk flag to refer to 10.4u
# instead.

# If you need to re-configure Qt, run "gmake confclean" in Qt source directory
# first.

cd $1
if [ "$2" == "universal" ]; then
    ./configure -platform macx-g++40 -release -opensource -no-exceptions -no-stl -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-declarative-debug -no-libtiff -no-libmng -no-openssl -nomake examples -nomake demos -no-dbus -no-dwarf2 -no-nis -universal -sdk /Developer/SDKs/MacOSX10.5.sdk # -carbon
else
    ./configure -platform macx-llvm -release -opensource -no-exceptions -no-stl -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-declarative-debug -no-libtiff -no-libmng -no-openssl -nomake examples -nomake demos -no-dbus -no-dwarf2 -no-nis -arch x86_64
fi

# When done, run the following commands to finish installation:
# make
# sudo make -j1 install

# When done, add the Qt binaries to your PATH environment variable
# For example, in ~/.bash_profile add something like:

# PATH=/usr/local/Trolltech/Qt-4.8.2/bin:$PATH
# export PATH
