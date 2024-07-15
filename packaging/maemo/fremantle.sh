#!/bin/sh
set -ex

VERSION=`cat packaging/version_number`
DEST=build/fremantle/portabase-$VERSION

rm -rf build/fremantle
packaging/copy_source.sh --keep-help $DEST/src
cd $DEST
mv src/portabase.pro src/src.pro
cp src/packaging/maemo/portabase.pro portabase.pro
cp -R src/packaging/maemo/debian .
if [ "$1" == "--source" ]; then
    dpkg-buildpackage -rfakeroot -sa -S
else
    RANLIB=ranlib dpkg-buildpackage -rfakeroot -uc -us -sa
fi
