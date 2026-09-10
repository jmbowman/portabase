# QtTest suite for the Edit Columns operation's resilience to inconsistent or
# corrupt file states (missing/orphaned "_all" entries, non-consecutive row
# IDs). Kept separate from tests/dbeditor/ because the crash-reproducing cases
# run in a child process, so a regression fails cleanly instead of segfaulting
# the whole run.

TEMPLATE = app
CONFIG  += qt testcase
QT      += testlib
include(../../common.pri)

INCLUDEPATH += $$PWD/../../src \
               $$PWD/../../metakit/include \
               $$PWD/../common
TARGET   = dbeditor_robustness_test
HEADERS += ../common/dbeditordriver.h \
           ../common/pobcorruptor.h \
           ../common/testfixtures.h
SOURCES += dbeditor_robustness_test.cpp \
           ../common/dbeditordriver.cpp \
           ../common/pobcorruptor.cpp \
           ../common/testfixtures.cpp
