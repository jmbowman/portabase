#!/bin/sh

KEEP_HELP="no"
if [ "$1" = "--keep-help" ]; then
    KEEP_HELP="yes"
    shift
fi

DEST=$1

rm -rf $DEST
mkdir -p $DEST/.tx
cp CHANGES $DEST
cp COPYING $DEST
cp Doxyfile $DEST
cp Doxyfile_Metakit $DEST
cp INSTALL $DEST
cp README.txt $DEST
cp metakit-2.4.9.8.patch $DEST
cp portabase.ico $DEST
cp portabase.iss $DEST
cp portabase.pro $DEST
cp portabase.rc $DEST
cp .tx/config $DEST/.tx
cp -R debian $DEST
mkdir $DEST/doc
cp doc/*.txt $DEST/doc
cp -R metakit $DEST
rm -f $DEST/metakit/builds/*.a
rm -f $DEST/metakit/builds/*.o
rm -f $DEST/metakit/builds/config.*
rm -f $DEST/metakit/builds/Makefile
cp -R packaging $DEST
cp -R resources $DEST
rm -f $DEST/resources/translations/*.qm
mkdir $DEST/src
cp src/*.h $DEST/src
cp src/*.cpp $DEST/src
cp -R src/calc $DEST/src
cp -R src/color_picker $DEST/src
cp -R src/encryption $DEST/src
cp -R src/image $DEST/src
cp -R src/qqutil $DEST/src
if [ "$KEEP_HELP" = "no" ]; then
    rm -rf $DEST/resources/help/_build
fi
find $DEST -name '*.DS_Store' -type f | xargs rm -f
