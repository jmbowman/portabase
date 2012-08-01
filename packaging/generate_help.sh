#!/bin/sh

# Script for generating the help files for a single language, the ISO code for
# which is passed as a parameter.  Useful for translators, and is used by the
# build script.  Run it from the PortaBase source code root directory.  To
# download the latest translations from Transifex before generating the help
# files, pass "update" as a second parameter.

# The main page of the generated output will be at
# packaging/resources/help/_build/index.html

cd resources/help
if [ "$1" = "--no-clean" ]; then
    shift 1
else
    make clean
fi
HELP_LANG="$1"
if [ -z "$HELP_LANG" ]; then
    HELP_LANG="en"
fi
if [ "$2" = "update" ]; then
    tx pull --mode developer -f -l $HELP_LANG
fi
if [ -d "translations/$HELP_LANG" ]; then
    cd "translations/$HELP_LANG"
    mkdir -p LC_MESSAGES
    for filename in `ls *.po`
    do
        base=$(basename "$filename" .po)
        msgfmt "$base.po" -o "LC_MESSAGES/$base.mo"
    done
    cd ../..
fi
HELP_LANG="$HELP_LANG" make html
HELP_LANG="$HELP_LANG" make text
mv _build/text/*.txt _build/html/_sources
rm -rf "translations/$HELP_LANG/LC_MESSAGES"
cd ../..
