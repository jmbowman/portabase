#!/usr/bin/env bash

# Build Metakit under MSYS2 on Windows, using the same mingw64 toolkit
# distributed with Qt which will be used to compile PortaBase.  Takes
# the mingw64 binaries directory as input, in native Windows path format
# ("C:\Qt\...").

export MINGW_BIN_DIR=cygpath -u "$1"
export PATH="$MINGW_BIN_DIR":$PATH
echo $PATH
cd "$(dirname "$0")"/../../metakit/builds
../unix/configure --enable-threads --disable-shared
mingw32-make
