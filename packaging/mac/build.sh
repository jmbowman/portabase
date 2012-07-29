#!/bin/sh

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory
# Takes the parameter to make -j as an argument, for utilizing multiple cores

# By default, builds a 64-bit Intel binary.  If you want to build a universal
# binary that runs on PowerPC and 32-bit Intel Macs, pass the string
# "universal" as the second parameter.  You'll need to have a suitably old or
# hacked version of Xcode, and appropriately compiled versions of Qt and
# Metakit.

VERSION=`cat packaging/version_number`
DIRNAME=PortaBase_$VERSION

# remove old build files
rm -rf build/*.app
rm -f build/*.dmg
rm -rf build/$DIRNAME
rm -rf resources/help/_build/_static # in case created for Maemo

# compile and make the application bundle
if [ "$2" == "universal" ]; then
    qmake -spec macx-g++40 portabase.pro
else
    qmake -spec macx-llvm portabase.pro
fi
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
cd ..
packaging/generate_help.sh en
sed -i '' 's:_static:../_static:g' resources/help/_build/html/*.html
mv resources/help/_build/html/_static build/PortaBase.app/Contents/Resources/
cp -R resources/help/_build/html/* build/PortaBase.app/Contents/Resources/en.lproj/
for dir in `ls resources/help/translations`
do
    if [ "$dir" == "templates" ]; then
        continue
    fi
    if [ -d "resources/help/translations/$dir" ]; then
        packaging/generate_help.sh "$dir"
        rm -r resources/help/_build/html/_static
        sed -i '' 's:_static:../_static:g' resources/help/_build/html/*.html
        target="$dir"
        if [ $dir = "zh_TW" ]; then
            target="zh-Hant"
        fi
        cp -R resources/help/_build/html/* "build/PortaBase.app/Contents/Resources/$target.lproj/"
    fi
done
cd build

# make the disk image for distribution
mkdir $DIRNAME
mv PortaBase.app $DIRNAME
cp ../README.txt $DIRNAME/ReadMe
cp ../CHANGES $DIRNAME/Changes
cp ../COPYING $DIRNAME/License
hdiutil create $DIRNAME.dmg -srcfolder $DIRNAME -format UDZO -volname $DIRNAME

# To verify no inappropriate library dependencies remain:
otool -L $DIRNAME/PortaBase.app/Contents/MacOS/PortaBase
