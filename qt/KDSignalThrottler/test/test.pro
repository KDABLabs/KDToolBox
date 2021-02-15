TEMPLATE = app

SOURCES += \
    tst_KDSignalThrottler.cpp \
    ../src/KDSignalThrottler.cpp

HEADERS += \
    ../src/KDSignalThrottler.h

INCLUDEPATH += \
    ../src/

QT = core testlib
CONFIG += testcase c++14 strict_c++
