#!/bin/sh

# Generate the help files in the host environment before running diablo.sh or
# fremantle.sh; it isn't worth setting up Sphinx in each Scratchbox environment

platform=$(uname)
cd resources/help
rm -rf _static
for dir in `ls`
do
    if [ -d "$dir" ]; then
        cd $dir
        make clean
        make html
        if [ "$platform" == "Darwin" ]; then
            sed -i '' 's:_static:../_static:g' _build/html/*.html
        else
            sed -i 's:_static:../_static:g' _build/html/*.html
        fi
        if [ $dir = "en" ]; then
            mv _build/html/_static ..
        else
            rm -r _build/html/_static
        fi
        cd ..
    fi
done
cd ../..
