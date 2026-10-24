#!/bin/sh

# Configure and compile Metakit as a static library in metakit/builds.  Pass
# "--check" to instead build it with Metakit's internal assertions enabled, in
# metakit/builds/check; the test suites link against that build (see
# docs/wiki/metakit-tips.md).

set -ex

cd "$(dirname "$0")"/../../metakit/builds
CONFIGURE=../unix/configure
if [ "$1" = "--check" ]; then
    export CPPFLAGS="-Dq4_CHECK $CPPFLAGS"
    mkdir -p check
    cd check
    CONFIGURE=../../unix/configure
fi
$CONFIGURE --enable-threads --disable-shared
make clean
make
