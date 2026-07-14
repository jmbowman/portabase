#!/bin/sh

# To be run from the project root, like "packaging/cppcheck/cppcheck.sh"
# Assumes that cppcheck has already been added to the PATH

set -ex

cppcheck --enable=all --inconclusive --library=qt --suppressions-list=packaging/cppcheck/suppressions.txt -q -isrc/encryption/randomkit src
