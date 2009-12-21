#!/bin/sh
#
# Build the application bundle for Mac OS X, after the executable has been
# compiled.

# Basic directory structure
mkdir PortaBase.app
mkdir PortaBase.app/Contents
mkdir PortaBase.app/Contents/MacOS
mkdir PortaBase.app/Contents/Frameworks
mkdir PortaBase.app/Contents/Resources

# Metadata files
cp Info.plist PortaBase.app/Contents/
cp PkgInfo PortaBase.app/Contents/

# Metakit shared library
cp /usr/local/lib/libmk4.dylib PortaBase.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/libmk4.dylib \
    PortaBase.app/Contents/Frameworks/libmk4.dylib

# Executable
cp ../portabase.app/Contents/MacOS/portabase PortaBase.app/Contents/MacOS/
install_name_tool -change libmk4.dylib \
    @executable_path/../Frameworks/libmk4.dylib \
    PortaBase.app/Contents/MacOS/PortaBase

# Icons
cp PortaBase.icns PortaBase.app/Contents/Resources/
cp PortaBaseFile.icns PortaBase.app/Contents/Resources/
mkdir PortaBase.app/Contents/Resources/icons
cp icons/*.png PortaBase.app/Contents/Resources/icons/
mkdir PortaBase.app/Contents/Resources/icons/portabase
cp icons/portabase/*.png PortaBase.app/Contents/Resources/icons/portabase/
cp icons/portabase/*.xpm PortaBase.app/Contents/Resources/icons/portabase/

# Help file (and translations thereof...use English when not translated)
mkdir PortaBase.app/Contents/Resources/cs.lproj
cp ../help/html/portabase.html PortaBase.app/Contents/Resources/cs.lproj/
mkdir PortaBase.app/Contents/Resources/de.lproj
cp ../help/html/portabase.html PortaBase.app/Contents/Resources/de.lproj/
mkdir PortaBase.app/Contents/Resources/en.lproj
cp ../help/html/portabase.html PortaBase.app/Contents/Resources/en.lproj/
mkdir PortaBase.app/Contents/Resources/fr.lproj
cp ../help/html/portabase.html PortaBase.app/Contents/Resources/fr.lproj/
mkdir PortaBase.app/Contents/Resources/ja.lproj
cp ../help/ja/html/portabase.html PortaBase.app/Contents/Resources/ja.lproj/
mkdir PortaBase.app/Contents/Resources/zh_TW.lproj
cp ../help/tw/html/portabase.html PortaBase.app/Contents/Resources/zh_TW.lproj/

# UI translation files
cd ..
lrelease portabase.pro
cd mac
cp ../portabase_cs.qm PortaBase.app/Contents/Resources/cs.lproj/
cp ../portabase_de.qm PortaBase.app/Contents/Resources/de.lproj/
cp ../portabase_fr.qm PortaBase.app/Contents/Resources/fr.lproj/
cp ../portabase_ja.qm PortaBase.app/Contents/Resources/ja.lproj/
cp ../portabase_zh_TW.qm PortaBase.app/Contents/Resources/zh_TW.lproj/
