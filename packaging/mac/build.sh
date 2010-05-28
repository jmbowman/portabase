#!/bin/sh

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory

VERSION=2.0rc1
DIRNAME=PortaBase_$VERSION

rm -rf build/*.app
rm -f build/*.dmg
rm -rf build/$DIRNAME
qmake -spec macx-g++40 portabase.pro
make clean
lrelease portabase.pro
lrelease resources/translations/qt*.ts
make
cd build
macdeployqt PortaBase.app
mkdir $DIRNAME
mv PortaBase.app $DIRNAME
cp ../README.txt $DIRNAME/ReadMe
cp ../CHANGES $DIRNAME/Changes
cp ../COPYING $DIRNAME/License
hdiutil create $DIRNAME.dmg -srcfolder $DIRNAME -format UDZO -volname $DIRNAME

# To verify no inappropriate library dependencies remain:
otool -L $DIRNAME/PortaBase.app/Contents/MacOS/PortaBase
