QT = core gui testlib
CONFIG += c++17 testcase
TEMPLATE = app
SOURCES += \
    tst_qt_fmt.cpp

INCLUDEPATH += \
    $$PWD/../fmt/include/

SOURCES += \
    $$PWD/../fmt/src/format.cc
