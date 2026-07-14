# QtTest suite covering DBEditor's Edit Columns dialog behavior.

TEMPLATE = app
CONFIG  += qt testcase
QT      += testlib
include(../../common.pri)

INCLUDEPATH += $$PWD/../../src
TARGET   = dbeditor_test
SOURCES += dbeditor_test.cpp
