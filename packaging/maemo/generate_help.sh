#!/bin/sh

# Generate the help files in the host environment before running diablo.sh or
# fremantle.sh; not worth setting up Sphinx in each Scratchbox environment

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
