#!/bin/sh

VERSION=`cat packaging/version_number`
DEST=build/debian/portabase-$VERSION

rm -rf build/debian
packaging/copy_source.sh $DEST
cd $DEST

# generate the help files here, since even unstable uses an outdated Sphinx
cd resources/help
for dir in `ls`
do
    if [ -d "$dir" ]; then
        cd $dir
        make html
        sed -i 's:_static:../_static:g' _build/html/*.html
        if [ $dir = "en" ]; then
            mv _build/html/_static ..
        else
            rm -r _build/html/_static
        fi
        cd ..
    fi
done
cd ../..

dpkg-buildpackage -rfakeroot -uc -us -sa
