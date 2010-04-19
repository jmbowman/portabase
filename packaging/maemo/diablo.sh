#!/bin/sh

VERSION=2.0
DEST=build/diablo/portabase-$VERSION

packaging/copy_source.sh $DEST/src
cd $DEST
mv src/portabase.pro src/src.pro
cp src/packaging/maemo/portabase.pro portabase.pro
cp -R src/packaging/maemo/debian .
cp src/packaging/maemo/diablo_control debian/control
cp src/packaging/maemo/diablo_postinst debian/postinst
dpkg-buildpackage -rfakeroot -uc -us -sa
