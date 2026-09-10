# QtTest suite covering the Edit Columns operation's behavior on valid files.
# Resilience to inconsistent/corrupt file states lives in
# tests/dbeditor_robustness/ instead.

TEMPLATE = app
CONFIG  += qt testcase
QT      += testlib
include(../../common.pri)

INCLUDEPATH += $$PWD/../../src \
               $$PWD/../../metakit/include \
               $$PWD/../common
TARGET   = dbeditor_test
HEADERS += ../common/dbeditordriver.h \
           ../common/testfixtures.h
SOURCES += dbeditor_test.cpp \
           ../common/dbeditordriver.cpp \
           ../common/testfixtures.cpp
