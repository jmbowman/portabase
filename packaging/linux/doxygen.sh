#!/bin/sh
set -ex

mkdir -p doc/html/metakit/html
doxygen Doxyfile_Metakit
doxygen Doxyfile
