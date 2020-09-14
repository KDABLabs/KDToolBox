TEMPLATE = app
TARGET = tst_qt_hasher
QT = core testlib gui
CONFIG += testcase c++11 strict_c++

SOURCES += \
    tst_qt_hasher.cpp

HEADERS += \
    tst_qt_hasher.h \
    ../qt_hasher.h
