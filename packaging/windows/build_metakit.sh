#!/usr/bin/env bash

cd "$(dirname "$0")"/../../metakit/builds
../unix/configure --enable-threads --disable-shared
make
