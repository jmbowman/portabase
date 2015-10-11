REM To be run from the project root, like "packaging\cppcheck\cppcheck.bat"
REM Assumes that cppcheck has already been added to the PATH

cppcheck --enable=all --inconclusive --suppressions-list=packaging\cppcheck\suppressions.txt -q -isrc\encryption\randomkit src
