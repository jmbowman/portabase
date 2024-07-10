#!/bin/sh
#
# (c) 2010-2016,2020 by Jeremy Bowman <jmbowman@alum.mit.edu>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Script for packaging the Mac version of PortaBase for distribution
# Assumes that we are in the PortaBase source code root directory
# Takes the parameter to make -j as an argument, for utilizing multiple cores

# This script builds a binary for the build system's native CPU architecture;
# support for PowerPC and 32-bit Intel Macs was dropped with the upgrade to
# Qt 5

VERSION=`cat packaging/version_number`
DIRNAME=PortaBase_$VERSION

# remove old build files
rm -rf build/*.app
rm -f build/*.dmg
rm -rf build/$DIRNAME
rm -rf resources/help/_build/_static # in case created for Maemo

# compile and make the application bundle
qmake -spec macx-clang portabase.pro
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

# prune unused image plugins
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqwebp.dylib
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqtiff.dylib
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqicns.dylib
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqmacjp2.dylib
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqtga.dylib
rm -f PortaBase.app/Contents/PlugIns/imageformats/libqwbmp.dylib

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
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtDBus.framework/Versions/5/QtDBus
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtMacExtras.framework/Versions/5/QtMacExtras
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtSvg.framework/Versions/5/QtSvg
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/Frameworks/QtXml.framework/Versions/5/QtXml
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/iconengines/libqsvgicon.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqgif.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqico.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqjpeg.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/imageformats/libqmacheif.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/platforms/libqcocoa.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app/Contents/PlugIns/styles/libqmacstyle.dylib
    codesign -s "Developer ID Application: Jeremy Bowman" PortaBase.app
fi

# make the disk image for distribution
mkdir $DIRNAME
mv PortaBase.app $DIRNAME
cp ../README.rst $DIRNAME/ReadMe
cp ../CHANGES $DIRNAME/Changes
cp ../COPYING $DIRNAME/License
DMGNAME=$DIRNAME.dmg
hdiutil create $DMGNAME -srcfolder $DIRNAME -format UDZO -volname $DIRNAME
if [ "$SIGN" == "Yes" ]; then
    codesign -s "Developer ID Application: Jeremy Bowman" $DMGNAME
fi

# To verify no inappropriate library dependencies remain:
otool -L $DIRNAME/PortaBase.app/Contents/MacOS/PortaBase
