#!/bin/sh

# To be run from the root directory of the PortaBase source code, as
# packaging/linux/debian.sh

# Generates a binary package by default.  To instead create a source package
# (for example, to upload to an autobuilder), add the "--source" parameter.

VERSION=`cat packaging/version_number`
DEST=build/debian/portabase-$VERSION

rm -rf build/debian
packaging/copy_source.sh $DEST
cd $DEST

if [ "$1" = "--source" ]; then
    dpkg-buildpackage -rfakeroot -sa -S
else
    dpkg-buildpackage -rfakeroot -uc -us -sa
fi
