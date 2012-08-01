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

# generate the help files here, since even unstable uses an outdated Sphinx
rm -rf resources/help/_build/_static
packaging/generate_help.sh en
cd resources/help/_build
sed -i 's:_static:../_static:g' html/*.html
mv html/_static .
mv html en
cd ../../..
for dir in `ls resources/help/translations`
do
    if [ "$dir" = "templates" ]; then
        continue
    fi
    if [ -d "resources/help/translations/$dir" ]; then
        packaging/generate_help.sh --no-clean "$dir"
        cd resources/help/_build
        sed -i 's:_static/translations:translations:g' html/*.html
        sed -i 's:_static:../_static:g' html/*.html
        mv html/_static/translations.js html/
        rm -r html/_static
        mv html "$dir"
        cd ../../..
    fi
done

if [ "$1" = "--source" ]; then
    dpkg-buildpackage -rfakeroot -sa -S
else
    dpkg-buildpackage -rfakeroot -uc -us -sa
fi
