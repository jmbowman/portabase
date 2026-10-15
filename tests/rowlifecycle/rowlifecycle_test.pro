# QtTest suite for the row lifecycle: adding, copying, and deleting data rows
# and the _id assignment they depend on (docs/intent/row-lifecycle/).

TEMPLATE = app
CONFIG  += qt testcase
QT      += testlib
include(../../common.pri)

INCLUDEPATH += $$PWD/../../src \
               $$PWD/../../metakit/include \
               $$PWD/../common
TARGET   = rowlifecycle_test
HEADERS += ../common/pobcorruptor.h \
           ../common/testfixtures.h
SOURCES += rowlifecycle_test.cpp \
           ../common/pobcorruptor.cpp \
           ../common/testfixtures.cpp
