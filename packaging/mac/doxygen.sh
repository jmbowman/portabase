#!/bin/sh

rm -rf doc/html
mkdir -p doc/html/metakit/html
doxygen Doxyfile_Metakit
doxygen Doxyfile
