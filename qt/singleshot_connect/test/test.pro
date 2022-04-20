TEMPLATE = app
TARGET = tst_singleshot_connect
QT = core testlib
CONFIG += testcase c++17

SOURCES += \
    tst_singleshot_connect.cpp \

HEADERS += \
    ../singleshot_connect.h
