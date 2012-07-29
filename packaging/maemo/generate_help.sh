#!/bin/sh

# Generate the help files in the host environment before running diablo.sh or
# fremantle.sh; it isn't worth setting up Sphinx in each Scratchbox environment

platform=$(uname)
rm -rf resources/help/_build/_static
packaging/generate_help.sh en
if [ "$platform" == "Darwin" ]; then
    sed -i '' 's:_static:../_static:g' resources/help/_build/html/*.html
else
    sed -i 's:_static:../_static:g' resources/help/_build/html/*.html
fi
mv resources/help/_build/html/_static resources/help/_build
mv resources/help/_build/html resources/help/_build/en
for dir in `ls resources/help/translations`
do
    if [ "$dir" == "templates" ]; then
        continue
    fi
    if [ -d "$dir" ]; then
        packaging/generate_help.sh "$dir"
        if [ "$platform" == "Darwin" ]; then
            sed -i '' 's:_static:../_static:g' resources/help/_build/html/*.html
        else
            sed -i 's:_static:../_static:g' resources/help/_build/html/*.html
        fi
        rm -r resources/help/_build/html/_static
        mv resources/help/_build/html "resources/help/_build/$dir"
    fi
done
