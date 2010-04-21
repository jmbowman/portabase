#!/bin/sh

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory

rm -f build/PortaBase.app
rm -f build/PortaBase.dmg
qmake -spec macx-g++40 portabase.pro
make clean
make
cd build
macdeployqt PortaBase.app
hdiutil create PortaBase.dmg -srcfolder PortaBase.app -format UDZO -volname PortaBase

# To verify no inappropriate library dependencies remain:
otool -L PortaBase.app/Contents/MacOS/PortaBase
