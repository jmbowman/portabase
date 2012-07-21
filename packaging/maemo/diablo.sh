#!/bin/sh

VERSION=`cat packaging/version_number`
DEST=build/diablo/portabase-$VERSION

rm -rf build/diablo
packaging/copy_source.sh --keep-help $DEST/src
cd $DEST
mv src/portabase.pro src/src.pro
cp src/packaging/maemo/portabase.pro portabase.pro
cp -R src/packaging/maemo/debian .
cp src/packaging/maemo/diablo_control debian/control
cp src/packaging/maemo/diablo_dirs debian/dirs
cp src/packaging/maemo/diablo_install debian/install
cp src/packaging/maemo/diablo_postinst debian/postinst
if [ "$1" == "source" ]; then
    dpkg-buildpackage -rfakeroot -sa -S
else
    dpkg-buildpackage -rfakeroot -uc -us -sa
fi
