#!/bin/sh

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory
# Takes the parameter to make -j as an argument, for utilizing multiple cores

# By default, builds a 64-bit Intel binary.  If you want to build a universal
# binary that runs on PowerPC and 32-bit Intel Macs, pass the string
# "--universal" as the first parameter.  You'll need to have a suitably old or
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
if [ "$1" == "--universal" ]; then
    shift 1
    qmake -spec macx-g++40 portabase.pro
else
    qmake -spec macx-llvm portabase.pro
fi
if [ "$1" == "--sign" ]; then
    SIGN=Yes
    shift 1
else
    SIGN=No
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
cd resources/help/_build
sed -i '' 's:_static:../_static:g' html/*.html
mv html/_static ../../../build/PortaBase.app/Contents/Resources/
cp -R html/* ../../../build/PortaBase.app/Contents/Resources/en.lproj/
cd ../../..
for dir in `ls resources/help/translations`
do
    if [ "$dir" == "templates" ]; then
        continue
    fi
    if [ -d "resources/help/translations/$dir" ]; then
        packaging/generate_help.sh --no-clean "$dir"
        cd resources/help/_build
        sed -i '' 's:_static/translations:translations:g' html/*.html
        sed -i '' 's:_static:../_static:g' html/*.html
        mv html/_static/translations.js html/
        rm -r html/_static
        target="$dir"
        if [ $dir = "zh_CN" ]; then
            target="zh-Hans"
        fi
        if [ $dir = "zh_TW" ]; then
            target="zh-Hant"
        fi
        cp -R html/* "../../../build/PortaBase.app/Contents/Resources/$target.lproj/"
        cd ../../..
    fi
done
cd build

# sign the binaries if the "--sign" parameter was passed in
if [ "$SIGN" == "Yes" ]; then
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/accessible/libqtaccessiblewidgets.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/bearer/libqcorewlanbearer.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/bearer/libqgenericbearer.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/codecs/libqcncodecs.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/codecs/libqjpcodecs.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/codecs/libqkrcodecs.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/codecs/libqtwcodecs.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/graphicssystems/libqtracegraphicssystem.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqgif.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqico.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqjpeg.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqtga.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app
fi

# make the disk image for distribution
mkdir $DIRNAME
mv PortaBase.app $DIRNAME
cp ../README.txt $DIRNAME/ReadMe
cp ../CHANGES $DIRNAME/Changes
cp ../COPYING $DIRNAME/License
hdiutil create $DIRNAME.dmg -srcfolder $DIRNAME -format UDZO -volname $DIRNAME
if [ "$SIGN" == "Yes" ]; then
    codesign -s "Developer ID Application: Jeremy Bowman" $DIRNAME.dmg
fi

# To verify no inappropriate library dependencies remain:
otool -L $DIRNAME/PortaBase.app/Contents/MacOS/PortaBase
