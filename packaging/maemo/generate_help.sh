#!/bin/sh

# Generate the help files in the host environment before running diablo.sh or
# fremantle.sh; it isn't worth setting up Sphinx in each Scratchbox environment

set -ex

platform=$(uname)
rm -rf resources/help/_build/_static
packaging/generate_help.sh en
cd resources/help/_build
if [ "$platform" = "Darwin" ]; then
    sed -i '' 's:_static:../_static:g' html/*.html
else
    sed -i 's:_static:../_static:g' html/*.html
fi
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
        if [ "$platform" = "Darwin" ]; then
            sed -i '' 's:_static/translations:translations:g' html/*.html
            sed -i '' 's:_static:../_static:g' html/*.html
        else
            sed -i 's:_static/translations:translations:g' html/*.html
            sed -i 's:_static:../_static:g' html/*.html
        fi
        mv html/_static/translations.js html/
        rm -r html/_static
        mv html "$dir"
        cd ../../..
    fi
done
