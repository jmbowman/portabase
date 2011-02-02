#!/bin/sh

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory
# Takes the parameter to make -j as an argument, for utilizing multiple cores

VERSION=2.0
DIRNAME=PortaBase_$VERSION

# remove old build files
rm -rf build/*.app
rm -f build/*.dmg
rm -rf build/$DIRNAME

# compile and make the application bundle
qmake -spec macx-g++40 portabase.pro
make clean
lrelease portabase.pro
lrelease resources/translations/qt*.ts
if [ -n "$1" ]; then
    make -j $1
else
    make
fi
if [ ! -f build/PortaBase.app/Contents/MacOS/PortaBase ]; then
    # build failed, fix that first
    exit 1
fi
cd build
macdeployqt PortaBase.app

# update and copy the help files
cd ../resources/help
for dir in `ls`
do
    if [ -d "$dir" ]; then
        cd $dir
        make clean
        make html
        sed -i '' 's:_static:../_static:g' _build/html/*.html
        target="$dir"
        if [ $dir = "zh_TW" ]; then
            target="zh-Hant"
            mv _build/html/_static ../../../build/PortaBase.app/Contents/Resources/
        else
            rm -r _build/html/_static
        fi
        cp -R _build/html/* "../../../build/PortaBase.app/Contents/Resources/$target.lproj/"
        cd ..
    fi
done
cd ../../build

# make the disk image for distribution
mkdir $DIRNAME
mv PortaBase.app $DIRNAME
cp ../README.txt $DIRNAME/ReadMe
cp ../CHANGES $DIRNAME/Changes
cp ../COPYING $DIRNAME/License
hdiutil create $DIRNAME.dmg -srcfolder $DIRNAME -format UDZO -volname $DIRNAME

# To verify no inappropriate library dependencies remain:
otool -L $DIRNAME/PortaBase.app/Contents/MacOS/PortaBase
