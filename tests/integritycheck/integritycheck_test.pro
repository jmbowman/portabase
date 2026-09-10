# QtTest suite for the IntegrityCheck scanner (src/integritycheck.*).
# The checker (src/integritycheck.cpp) and the Database API used by the
# fixture builder both come in via common.pri, so they are not listed here.

TEMPLATE = app
CONFIG  += qt testcase
QT      += testlib
include(../../common.pri)

INCLUDEPATH += $$PWD/../../src \
               $$PWD/../../metakit/include \
               $$PWD/../common
TARGET   = integritycheck_test
HEADERS += ../common/pobcorruptor.h \
           ../common/testfixtures.h
SOURCES += integritycheck_test.cpp \
           ../common/pobcorruptor.cpp \
           ../common/testfixtures.cpp
