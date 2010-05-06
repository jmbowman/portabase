#!/bin/sh

VERSION=2.0~beta1
DEST=build/fremantle/portabase-$VERSION

rm -rf build/fremantle
packaging/copy_source.sh $DEST/src
cd $DEST
mv src/portabase.pro src/src.pro
cp src/packaging/maemo/portabase.pro portabase.pro
cp -R src/packaging/maemo/debian .
dpkg-buildpackage -rfakeroot -sa -S
#dpkg-buildpackage -rfakeroot -uc -us -sa
