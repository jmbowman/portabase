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
sed -i 's:_static:../_static:g' resources/help/_build/html/*.html
mv resources/help/_build/html/_static resources/help/_build
mv resources/help/_build/html resources/help/_build/en
for dir in `ls resources/help/translations`
do
    if [ "$dir" == "templates" ]; then
        continue
    fi
    if [ -d "$dir" ]; then
        packaging/generate_help.sh "$dir"
        sed -i 's:_static:../_static:g' resources/help/_build/html/*.html
        rm -r resources/help/_build/html/_static
        mv resources/help/_build/html "resources/help/_build/$dir"
    fi
done

if [ "$1" == "--source" ]; then
    dpkg-buildpackage -rfakeroot -sa -S
else
    dpkg-buildpackage -rfakeroot -uc -us -sa
fi
