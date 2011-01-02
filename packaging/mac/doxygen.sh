#!/bin/sh

mkdir -p doc/html/metakit/html
/Applications/Doxygen.app/Contents/Resources/doxygen Doxyfile_Metakit
/Applications/Doxygen.app/Contents/Resources/doxygen Doxyfile
