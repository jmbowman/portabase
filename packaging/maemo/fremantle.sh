#!/bin/sh

VERSION=2.0
DEST=build/diablo/portabase-$VERSION

mkdir -p $DEST
cd $DEST
git clone ../../../ src
rm -r src/.git
mv src/portabase.pro src/src.pro
cp src/packaging/maemo/portabase.pro portabase.pro
